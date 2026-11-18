#include "complaints.h"
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

static void test_complaint_lifecycle(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F2", "D2", "R2");
    
    SessionPlacement live_placements[2];
    strcpy(live_placements[0].session_id, "S1");
    live_placements[0].assigned_slot = 0; strcpy(live_placements[0].assigned_room, "R1");
    strcpy(live_placements[1].session_id, "S2");
    live_placements[1].assigned_slot = 1; strcpy(live_placements[1].assigned_room, "R2");
    
    Room rooms[2] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, 0}; hash_map_put(&map, "F1", &f1);
    Faculty f2 = {"F2", 0, 0}; hash_map_put(&map, "F2", &f2);
    
    Complaint c;
    complaint_init(&c, "COMP-1", "S1", COMPLAINT_GAP, "S1 has a gap");
    assert(c.status == STATUS_PENDING);
    
    assert(complaint_generate_proposal(&c, sessions, 2, rooms, 2, &map, live_placements) == 0);
    assert(c.status == STATUS_PROPOSED);
    assert(c.proposed_timetable != NULL);
    assert(c.proposed_timetable[0].assigned_slot != 0);
    
    complaint_approve(&c);
    assert(c.status == STATUS_APPROVED);
    
    assert(complaint_commit(&c, sessions, 2, rooms, 2, &map, live_placements) == 0);
    assert(c.status == STATUS_RESOLVED);
    
    assert(live_placements[0].assigned_slot != 0);
    
    complaint_free(&c);
    hash_map_free(&map);
}

static void test_complaint_drift_failure(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F2", "D2", "R2");
    
    SessionPlacement live_placements[2];
    strcpy(live_placements[0].session_id, "S1");
    live_placements[0].assigned_slot = 0; strcpy(live_placements[0].assigned_room, "R1");
    strcpy(live_placements[1].session_id, "S2");
    live_placements[1].assigned_slot = 1; strcpy(live_placements[1].assigned_room, "R2");
    
    Room rooms[2] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, 0}; hash_map_put(&map, "F1", &f1);
    Faculty f2 = {"F2", 0, 0}; hash_map_put(&map, "F2", &f2);
    
    Complaint c;
    complaint_init(&c, "COMP-2", "S1", COMPLAINT_SHIFT, "Shift S1");
    assert(complaint_generate_proposal(&c, sessions, 2, rooms, 2, &map, live_placements) == 0);
    complaint_approve(&c);
    
    sessions[0].unavail_count = 34;
    for (int i=1; i<35; i++) sessions[0].unavail[i-1] = i; 
    
    assert(complaint_commit(&c, sessions, 2, rooms, 2, &map, live_placements) == -1);
    assert(c.status == STATUS_FAILED);
    
    assert(live_placements[0].assigned_slot == 0);
    
    complaint_free(&c);
    hash_map_free(&map);
}

int main(void) {
    printf("=== M7: complaints tests ===\n");
    test_complaint_lifecycle();
    printf("  [PASS] test_complaint_lifecycle\n");
    test_complaint_drift_failure();
    printf("  [PASS] test_complaint_drift_failure\n");
    
    printf("\nAll M7 tests passed.\n");
    return 0;
}
