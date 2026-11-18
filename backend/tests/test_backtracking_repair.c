#include "backtracking_repair.h"
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

static void test_eviction_repair(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F1", "D2", "R2");
    
    sessions[0].unavail_count = 33;
    for (int i=2; i<35; i++) sessions[0].unavail[i-2] = i;
    
    sessions[1].unavail_count = 34;
    for (int i=1; i<35; i++) sessions[1].unavail[i-1] = i;
    
    Room rooms[2] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f = {"F1", 0, 0}; hash_map_put(&map, "F1", &f);
    
    SessionPlacement placements[2];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0;
    strcpy(placements[0].assigned_room, "R1");
    placements[0].blocking_reason[0] = '\0';
    
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = -1;
    strcpy(placements[1].blocking_reason, "Forced unplaced");
    
    assert(repair_timetable(sessions, 2, rooms, 2, &map, placements, 10) == 0);
    
    assert(placements[1].assigned_slot == 0);
    assert(placements[0].assigned_slot == 1);
    
    hash_map_free(&map);
}

int main(void) {
    printf("=== M4: backtracking_repair tests ===\n");
    test_eviction_repair();
    printf("  [PASS] test_eviction_repair\n");
    
    printf("\nAll M4 tests passed.\n");
    return 0;
}
