#include "faculty_unavail_repair.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void make_session(Session *s, const char *id,
                         const char *faculty, const char *division, const char *room) {
    session_init(s);
    strncpy(s->id,       id,       MAX_ID_LEN   - 1);
    strncpy(s->faculty,  faculty,  MAX_NAME_LEN - 1);
    strncpy(s->division, division, MAX_NAME_LEN - 1);
    strncpy(s->subject,  "Subj",   MAX_NAME_LEN - 1);
    s->student_count = 50;
    s->required_type = ROOM_CLASSROOM;
    s->allowed_rooms_count = 1;
    strcpy(s->allowed_rooms[0], room);
}

static void test_basic_unavail_repair(void) {
    Session sessions[3];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F2", "D2", "R2");
    make_session(&sessions[2], "S3", "F3", "D3", "R3");
    
    SessionPlacement placements[3];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1");
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 1; strcpy(placements[1].assigned_room, "R2");
    strcpy(placements[2].session_id, "S3");
    placements[2].assigned_slot = 2; strcpy(placements[2].assigned_room, "R3");
    
    Room rooms[3] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM}, {"R3", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, 0}; hash_map_put(&map, "F1", &f1);
    Faculty f2 = {"F2", 0, 0}; hash_map_put(&map, "F2", &f2);
    Faculty f3 = {"F3", 0, 0}; hash_map_put(&map, "F3", &f3);
    
    int new_unavail[] = {0};
    assert(mark_faculty_unavailable("F1", new_unavail, 1, sessions, 3, rooms, 3, &map, placements) == 0);
    
    assert(placements[0].assigned_slot != 0);
    assert(placements[0].assigned_slot != -1);
    
    Faculty *fac = (Faculty *)hash_map_get(&map, "F1");
    assert((fac->unavail_mask & (1ULL << 0)) != 0);
    
    hash_map_free(&map);
}

static void test_cascading_repair(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F2", "D1", "R2"); 
    
    sessions[0].unavail_count = 33;
    for (int i=2; i<35; i++) sessions[0].unavail[i-2] = i; 
    
    SessionPlacement placements[2];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1");
    
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 1; strcpy(placements[1].assigned_room, "R2");
    
    Room rooms[2] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, 0}; hash_map_put(&map, "F1", &f1);
    Faculty f2 = {"F2", 0, 0}; hash_map_put(&map, "F2", &f2);
    
    int new_unavail[] = {0};
    assert(mark_faculty_unavailable("F1", new_unavail, 1, sessions, 2, rooms, 2, &map, placements) == 0);
    
    assert(placements[0].assigned_slot == 1);
    assert(placements[1].assigned_slot != -1);
    assert(placements[1].assigned_slot != 1); 
    
    hash_map_free(&map);
}

int main(void) {
    printf("=== M6: faculty_unavail_repair tests ===\n");
    test_basic_unavail_repair();
    printf("  [PASS] test_basic_unavail_repair\n");
    test_cascading_repair();
    printf("  [PASS] test_cascading_repair\n");
    
    printf("\nAll M6 tests passed.\n");
    return 0;
}
