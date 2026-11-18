#include "slot_helpers.h"
#include "avail.h"
#include "hash_map.h"
#include "core_types.h"
#include "graph_builder.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_valid_start_table(void) {
    assert(valid_start(3, 2) == 0);
    assert(valid_start(2, 2) == 1);
    assert(valid_start(4, 2) == 1);
    assert(valid_start(6, 2) == 0);
    assert(valid_start(6, 1) == 1);
    printf("test_valid_start_table PASSED\n");
}

void test_placement_slots(void) {
    Session s; session_init(&s); s.duration = 2;
    SessionPlacement p; memset(&p, 0, sizeof(p));
    p.assigned_slot = 4;
    
    int slots[5];
    int n = placement_slots(&p, &s, slots);
    assert(n == 2);
    assert(slots[0] == 4 && slots[1] == 5);
    printf("test_placement_slots PASSED\n");
}

void test_consecutive_cap(void) {
    AvailTracker tr;
    avail_tracker_init(&tr, 31);
    HashMap map;
    hash_map_init(&map, 31);
    
    Faculty f = {"F1", 3, 0};
    hash_map_put(&map, "F1", &f);
    
    // Book slot 1 and 3 for F1
    avail_book(&tr.faculty, "F1", 1, "S_PREV");
    avail_book(&tr.faculty, "F1", 3, "S_NEXT");
    
    Session s; session_init(&s);
    strcpy(s.id, "S_NEW"); strcpy(s.faculty, "F1"); strcpy(s.division, "DivA");
    s.duration = 1;
    
    // Attempting to book slot 2 should merge them into run of 3, max is 3, so success.
    assert(try_place(&tr, &map, &s, 2, "Room1") == REASON_NONE);
    unplace(&tr, &s, 2, "Room1");
    
    // Now make max_consecutive = 2
    f.max_consecutive = 2;
    assert(try_place(&tr, &map, &s, 2, "Room1") == REASON_CONSECUTIVE_LIMIT);
    
    // What if it's across lunch?
    // Book slot 3 and try placing in slot 4 (periods 3 and 4)
    avail_book(&tr.faculty, "F1", 4, "S_NEXT");
    // Place S_NEW at slot 4... wait, slot 4 is booked.
    // Try placing at slot 4 with slot 3 booked: are they consecutive?
    // Let's clear tr
    avail_tracker_free(&tr);
    avail_tracker_init(&tr, 31);
    
    avail_book(&tr.faculty, "F1", 2, "S_PREV");
    avail_book(&tr.faculty, "F1", 3, "S_PREV2");
    // run length 2 (slots 2, 3)
    // Attempt to book slot 4. Are 3 and 4 consecutive? No.
    // So if we place 1hr class at slot 4, run length is 1. (max is 2).
    s.duration = 1;
    assert(try_place(&tr, &map, &s, 4, "Room1") == REASON_NONE);
    
    avail_tracker_free(&tr);
    hash_map_free(&map);
    printf("test_consecutive_cap PASSED\n");
}

void test_rollback(void) {
    AvailTracker tr;
    avail_tracker_init(&tr, 31);
    HashMap map;
    hash_map_init(&map, 31);
    
    Session s; session_init(&s);
    strcpy(s.id, "S_LAB"); strcpy(s.faculty, "F1"); strcpy(s.division, "DivA");
    s.duration = 2;
    
    // Book slot 5 for another session, meaning slot 4+5 (2 hours) will fail on slot 5
    avail_book(&tr.faculty, "F1", 5, "OTHER");
    
    // Copy the tracker state
     
    // Wait, simple struct copy works here? Yes, but pointers (buckets) are the same! 
    // It's shallow copy. So if it mutates, the backup's pointers point to the mutated data!
    // Since we know our try_place pre-checks, we can just assert it failed and 
    // then manually verify slot 4 is still free.
    
    assert(try_place(&tr, &map, &s, 4, "Room1") == REASON_NO_FREE_SLOT);
    
    // Verify slot 4 is free for everyone
    assert(!avail_is_booked(&tr.faculty, "F1", 4));
    assert(!avail_is_booked(&tr.room, "Room1", 4));
    assert(!avail_is_booked(&tr.division, "DivA", 4));
    
    avail_tracker_free(&tr);
    hash_map_free(&map);
    printf("test_rollback PASSED\n");
}

void test_faculty_mask(void) {
    AvailTracker tr; avail_tracker_init(&tr, 31);
    HashMap map; hash_map_init(&map, 31);
    
    Faculty f = {"F1", 0, (1ULL << 5)}; // Slot 5 unavailable
    hash_map_put(&map, "F1", &f);
    
    Session s; session_init(&s);
    strcpy(s.id, "S_LAB"); strcpy(s.faculty, "F1"); strcpy(s.division, "DivA");
    s.duration = 2;
    
    // Slot 4 and 5 -> 5 is blocked by mask
    assert(try_place(&tr, &map, &s, 4, "Room1") == REASON_FACULTY_UNAVAIL);
    
    // Slot 2 and 3 -> free
    assert(try_place(&tr, &map, &s, 2, "Room1") == REASON_NONE);
    
    avail_tracker_free(&tr);
    hash_map_free(&map);
    printf("test_faculty_mask PASSED\n");
}

void test_joint_session(void) {
    AvailTracker tr; avail_tracker_init(&tr, 31);
    HashMap map; hash_map_init(&map, 31);
    
    Session s; session_init(&s);
    strcpy(s.id, "S_JOINT"); strcpy(s.faculty, "F1"); 
    strcpy(s.division, "Joint");
    s.joint_divisions_count = 2;
    strcpy(s.joint_divisions[0], "DivA");
    strcpy(s.joint_divisions[1], "DivB");
    s.duration = 1;
    
    // Book DivB at slot 0
    avail_book(&tr.division, "DivB", 0, "OTHER");
    
    assert(try_place(&tr, &map, &s, 0, "Room1") == REASON_NO_FREE_SLOT);
    
    // Slot 1 is free for both
    assert(try_place(&tr, &map, &s, 1, "Room1") == REASON_NONE);
    assert(avail_is_booked(&tr.division, "DivA", 1));
    assert(avail_is_booked(&tr.division, "DivB", 1));
    
    avail_tracker_free(&tr);
    hash_map_free(&map);
    printf("test_joint_session PASSED\n");
}

void test_unplace(void) {
    AvailTracker tr; avail_tracker_init(&tr, 31);
    HashMap map; hash_map_init(&map, 31);
    
    Session s; session_init(&s);
    strcpy(s.id, "S1"); strcpy(s.faculty, "F1"); strcpy(s.division, "DivA");
    s.duration = 2;
    
    assert(try_place(&tr, &map, &s, 0, "Room1") == REASON_NONE);
    assert(avail_is_booked(&tr.faculty, "F1", 0));
    assert(avail_is_booked(&tr.faculty, "F1", 1));
    
    unplace(&tr, &s, 0, "Room1");
    assert(!avail_is_booked(&tr.faculty, "F1", 0));
    assert(!avail_is_booked(&tr.faculty, "F1", 1));
    assert(!avail_is_booked(&tr.division, "DivA", 0));
    assert(!avail_is_booked(&tr.room, "Room1", 1));
    
    avail_tracker_free(&tr);
    hash_map_free(&map);
    printf("test_unplace PASSED\n");
}

int main(void) {
    test_valid_start_table();
    test_placement_slots();
    test_consecutive_cap();
    test_rollback();
    test_faculty_mask();
    test_joint_session();
    test_unplace();
    printf("ALL PHASE 2 TESTS PASSED\n");
    return 0;
}
