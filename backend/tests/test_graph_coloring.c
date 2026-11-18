#include "graph_coloring.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void make_session(Session *s, const char *id,
                         const char *faculty, const char *division) {
    session_init(s);
    strncpy(s->id,       id,       MAX_ID_LEN   - 1);
    strncpy(s->faculty,  faculty,  MAX_NAME_LEN - 1);
    strncpy(s->division, division, MAX_NAME_LEN - 1);
    strncpy(s->subject,  "Subj",   MAX_NAME_LEN - 1);
    s->student_count = 50;
    s->required_type = ROOM_CLASSROOM;
}

static void test_basic_earliest(void) {
    Session sessions[3];
    make_session(&sessions[0], "S1", "F1", "D1");
    make_session(&sessions[1], "S2", "F2", "D1"); 
    make_session(&sessions[2], "S3", "F1", "D2"); 
    
    Room rooms[1] = { {"R1", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, 0}; hash_map_put(&map, "F1", &f1);
    Faculty f2 = {"F2", 0, 0}; hash_map_put(&map, "F2", &f2);
    
    SessionPlacement results[3];
    assert(color_timetable(sessions, 3, rooms, 1, &map, ORDERING_EARLIEST, results) == 0);
    
    assert(results[0].assigned_slot != -1);
    assert(results[1].assigned_slot != -1);
    assert(results[2].assigned_slot != -1);
    assert(results[0].assigned_slot != results[1].assigned_slot); // D1 conflicts
    assert(results[0].assigned_slot != results[2].assigned_slot); // F1 conflicts
    
    hash_map_free(&map);
}

static void test_fixed_slots(void) {
    Session sessions[3];
    make_session(&sessions[0], "S1", "F1", "D1");
    sessions[0].fixed_slot = 10;
    make_session(&sessions[1], "S2", "F1", "D2");
    make_session(&sessions[2], "S3", "F2", "D1");
    
    Room rooms[2] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, 0}; hash_map_put(&map, "F1", &f1);
    Faculty f2 = {"F2", 0, 0}; hash_map_put(&map, "F2", &f2);
    
    SessionPlacement results[3];
    assert(color_timetable(sessions, 3, rooms, 2, &map, ORDERING_EARLIEST, results) == 0);
    
    int s1_idx = -1;
    for (int i=0; i<3; i++) if (strcmp(results[i].session_id, "S1") == 0) s1_idx = i;
    
    assert(results[s1_idx].assigned_slot == 10);
    
    hash_map_free(&map);
}

static void test_cannot_place(void) {
    Session sessions[1];
    make_session(&sessions[0], "S1", "F1", "D1");
    
    Room rooms[1] = { {"R1", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    // Block all 35 slots
    Faculty f1 = {"F1", 0, 0x7FFFFFFFFULL}; 
    hash_map_put(&map, "F1", &f1);
    
    SessionPlacement results[1];
    assert(color_timetable(sessions, 1, rooms, 1, &map, ORDERING_EARLIEST, results) == 0);
    
    assert(results[0].assigned_slot == -1);
    assert(results[0].reason_code == REASON_FACULTY_UNAVAIL);
    
    hash_map_free(&map);
}

int main(void) {
    printf("=== M3: graph_coloring tests ===\n");
    test_basic_earliest();
    printf("  [PASS] test_basic_earliest\n");
    test_fixed_slots();
    printf("  [PASS] test_fixed_slots\n");
    test_cannot_place();
    printf("  [PASS] test_cannot_place\n");
    printf("\nAll M3 tests passed.\n");
    return 0;
}
