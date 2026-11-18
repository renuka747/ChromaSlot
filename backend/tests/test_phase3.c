#include "preflight.h"
#include "graph_builder.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_fit_room(void) {
    Room r = {"R1", 50, ROOM_CLASSROOM};
    Session s; session_init(&s);
    s.student_count = 60;
    s.required_type = ROOM_CLASSROOM;
    
    // Capacity fail
    assert(fit_room(&s, &r) == 0);
    
    s.student_count = 40;
    // Type fail
    s.required_type = ROOM_COMPUTER_LAB;
    assert(fit_room(&s, &r) == 0);
    
    s.required_type = ROOM_CLASSROOM;
    // Allowed rooms check
    s.allowed_rooms_count = 1;
    strcpy(s.allowed_rooms[0], "R2");
    assert(fit_room(&s, &r) == 0);
    
    strcpy(s.allowed_rooms[0], "R1");
    assert(fit_room(&s, &r) == 1);
    
    printf("test_fit_room PASSED\n");
}

void test_split_into_batches(void) {
    Room rooms[5] = {
        {"R101", 80, ROOM_CLASSROOM},
        {"R102", 60, ROOM_CLASSROOM},
        {"R103", 60, ROOM_CLASSROOM},
        {"Lab1", 40, ROOM_COMPUTER_LAB},
        {"Lab2", 40, ROOM_ELECTRONICS_LAB}
    };
    
    Session sessions[10]; int num_sessions = 4;
    session_init(&sessions[0]); strcpy(sessions[0].id, "ALab"); sessions[0].student_count = 65; sessions[0].required_type = ROOM_COMPUTER_LAB; sessions[0].splittable = 1;
    session_init(&sessions[1]); strcpy(sessions[1].id, "BLab"); sessions[1].student_count = 55; sessions[1].required_type = ROOM_COMPUTER_LAB; sessions[1].splittable = 1;
    session_init(&sessions[2]); strcpy(sessions[2].id, "BElec"); sessions[2].student_count = 55; sessions[2].required_type = ROOM_ELECTRONICS_LAB; sessions[2].splittable = 1;
    session_init(&sessions[3]); strcpy(sessions[3].id, "SOFT"); sessions[3].student_count = 120; sessions[3].required_type = ROOM_CLASSROOM; sessions[3].splittable = 0;
    
    split_into_batches(sessions, &num_sessions, rooms, 5);
    
    // Total should be: ALab (2) + BLab (2) + BElec (2) + SOFT (1) = 7
    assert(num_sessions == 7);
    
    // Find each batch and assert
    int a1=0, a2=0, b1=0, b2=0, e1=0, e2=0, soft=0;
    for (int i=0; i<num_sessions; i++) {
        if (strcmp(sessions[i].id, "ALab-B1") == 0) { assert(sessions[i].student_count == 33); a1++; }
        if (strcmp(sessions[i].id, "ALab-B2") == 0) { assert(sessions[i].student_count == 32); a2++; }
        if (strcmp(sessions[i].id, "BLab-B1") == 0) { assert(sessions[i].student_count == 28); b1++; }
        if (strcmp(sessions[i].id, "BLab-B2") == 0) { assert(sessions[i].student_count == 27); b2++; }
        if (strcmp(sessions[i].id, "BElec-B1") == 0) { assert(sessions[i].student_count == 28); e1++; }
        if (strcmp(sessions[i].id, "BElec-B2") == 0) { assert(sessions[i].student_count == 27); e2++; }
        if (strcmp(sessions[i].id, "SOFT") == 0) { assert(sessions[i].student_count == 120); soft++; }
    }
    assert(a1 == 1 && a2 == 1);
    assert(b1 == 1 && b2 == 1);
    assert(e1 == 1 && e2 == 1);
    assert(soft == 1);
    
    printf("test_split_into_batches PASSED\n");
}

void test_preflight_check(void) {
    Room rooms[1] = { {"R101", 80, ROOM_CLASSROOM} };
    Session sessions[3]; int num_sessions = 3;
    
    session_init(&sessions[0]); strcpy(sessions[0].id, "SOFT"); 
    sessions[0].student_count = 120; sessions[0].required_type = ROOM_CLASSROOM;
    
    session_init(&sessions[1]); strcpy(sessions[1].id, "CLASH1"); 
    sessions[1].student_count = 50; sessions[1].required_type = ROOM_CLASSROOM; 
    sessions[1].fixed_slot = 3; sessions[1].duration = 2; // crosses lunch (periods 3 and 4)
    
    session_init(&sessions[2]); strcpy(sessions[2].id, "CLASH2"); 
    sessions[2].student_count = 50; sessions[2].required_type = ROOM_CLASSROOM; 
    sessions[2].fixed_slot = 0; sessions[2].duration = 1; 
    strcpy(sessions[2].faculty, "F1");
    
    HashMap map; hash_map_init(&map, 31);
    Faculty f = {"F1", 0, 1ULL}; // Unavail slot 0
    hash_map_put(&map, "F1", &f);
    
    SessionPlacement placements[3];
    int fails = preflight_check(sessions, num_sessions, rooms, 1, &map, placements);
    
    assert(fails == 3);
    assert(placements[0].reason_code == REASON_CAPACITY);
    assert(placements[1].reason_code == REASON_FIXED_SLOT_CLASH);
    assert(placements[2].reason_code == REASON_FACULTY_UNAVAIL);
    
    hash_map_free(&map);
    printf("test_preflight_check PASSED\n");
}

int main(void) {
    test_fit_room();
    test_split_into_batches();
    test_preflight_check();
    printf("ALL PHASE 3 TESTS PASSED\n");
    return 0;
}
