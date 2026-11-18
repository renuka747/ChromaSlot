#include "health_check.h"
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
    s->duration = 1;
    s->required_type = ROOM_CLASSROOM;
    s->allowed_rooms_count = 1;
    strcpy(s->allowed_rooms[0], room);
}

static void test_healthy(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F2", "D2", "R2");
    
    SessionPlacement placements[2];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1");
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 1; strcpy(placements[1].assigned_room, "R2");
    
    Room rooms[2] = { {"R1", 100, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} };
    HashMap map; hash_map_init(&map, 31);
    
    HealthCheckReport r;
    run_health_check(sessions, 2, rooms, 2, &map, placements, &r);
    
    assert(r.total_issues == 0);
    hash_map_free(&map);
}

static void test_error_injection(void) {
    Session sessions[4];
    make_session(&sessions[0], "S1", "F1", "D1", "R1"); 
    sessions[0].duration = 2; // 2 hour slot
    
    make_session(&sessions[1], "S2", "F1", "D2", "R1"); // Conflicts with S1 on faculty, room
    make_session(&sessions[2], "S3", "F2", "D1", "R2"); // Conflicts with S1 on D1
    
    // Joint session
    make_session(&sessions[3], "S4", "F3", "D3", "R2");
    sessions[3].joint_divisions_count = 2;
    strcpy(sessions[3].joint_divisions[0], "D3");
    strcpy(sessions[3].joint_divisions[1], "D1"); // Conflicts with S3 on D1!
    
    SessionPlacement placements[4];
    
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1"); // Occupies 0, 1
    
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 1; strcpy(placements[1].assigned_room, "R1"); // Overlaps slot 1!
    
    strcpy(placements[2].session_id, "S3");
    placements[2].assigned_slot = 0; strcpy(placements[2].assigned_room, "R2"); // Overlaps slot 0!
    
    strcpy(placements[3].session_id, "S4");
    placements[3].assigned_slot = 0; strcpy(placements[3].assigned_room, "R2"); // Overlaps slot 0! (Room & Div conflict with S3)
    
    Room rooms[2] = { {"R1", 10, ROOM_CLASSROOM}, {"R2", 100, ROOM_CLASSROOM} }; // R1 capacity is 10 (S1, S2 are 50 -> capacity fail)
    
    HashMap map; hash_map_init(&map, 31);
    Faculty f1 = {"F1", 0, (1ULL << 0)}; // F1 is unavailable at slot 0
    hash_map_put(&map, "F1", &f1);
    
    HealthCheckReport r;
    run_health_check(sessions, 4, rooms, 2, &map, placements, &r);
    
    // 1. S1 vs S2: faculty (F1), room (R1)
    // 2. S3 vs S4: room (R2), division (D1)
    // 3. S1 vs S3: division (D1)
    // 4. S1 vs S4: division (D1)
    assert(r.faculty_conflicts == 1);
    assert(r.room_conflicts == 2);
    assert(r.division_conflicts == 3);
    
    // Capacity: S1 in R1 (fails), S2 in R1 (fails)
    assert(r.capacity_violations == 2);
    
    // Unavail: S1 at 0 (F1 mask)
    assert(r.unavail_violations == 1);
    
    hash_map_free(&map);
}

static void test_consecutive_and_unplaced(void) {
    Session sessions[4];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F1", "D1", "R2");
    make_session(&sessions[2], "S3", "F1", "D1", "R3");
    make_session(&sessions[3], "S4", "F2", "D2", "R4");
    
    SessionPlacement placements[4];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1");
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 1; strcpy(placements[1].assigned_room, "R2");
    strcpy(placements[2].session_id, "S3");
    placements[2].assigned_slot = 2; strcpy(placements[2].assigned_room, "R3");
    
    strcpy(placements[3].session_id, "S4");
    placements[3].assigned_slot = -1; strcpy(placements[3].assigned_room, "");
    
    Room rooms[4] = { {"R1",100,0}, {"R2",100,0}, {"R3",100,0}, {"R4",100,0} };
    HashMap map; hash_map_init(&map, 31);
    
    Faculty f1 = {"F1", 2, 0}; hash_map_put(&map, "F1", &f1);
    
    HealthCheckReport r;
    run_health_check(sessions, 4, rooms, 4, &map, placements, &r);
    
    assert(r.consecutive_violations == 2); // F1 has 1, D1 has 1
    assert(r.unplaced_sessions == 1);
    assert(r.total_issues == 3);
    
    hash_map_free(&map);
}

int main(void) {
    printf("=== M8: health_check tests ===\n");
    test_healthy();
    printf("  [PASS] test_healthy\n");
    test_error_injection();
    printf("  [PASS] test_error_injection\n");
    test_consecutive_and_unplaced();
    printf("  [PASS] test_consecutive_and_unplaced\n");
    
    printf("\nAll M8 tests passed.\n");
    return 0;
}
