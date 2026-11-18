#include "json_loader.h"
#include "db.h"
#include "core_types.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void assert_adversarial_dataset(void) {
    unlink("test_adversarial.db");
    assert(db_init("test_adversarial.db") == 0);
    assert(load_adversarial_dataset("tests/data/adversarial.json") == 0);
    
    Room rooms[100]; int num_rooms = 0;
    db_load_rooms(rooms, &num_rooms);
    printf("- 5 rooms, 10 faculty, 2 divisions (A=65, B=55)\n");
    printf("  Rooms found: %d\n", num_rooms);
    assert(num_rooms == 5);
    
    int r101=0, r102=0, r103=0, lab1=0, lab2=0;
    for (int i=0; i<num_rooms; i++) {
        if (strcmp(rooms[i].id, "R101") == 0) r101 = rooms[i].type;
        if (strcmp(rooms[i].id, "R102") == 0) r102 = rooms[i].type;
        if (strcmp(rooms[i].id, "R103") == 0) r103 = rooms[i].type;
        if (strcmp(rooms[i].id, "Lab1") == 0) lab1 = rooms[i].type;
        if (strcmp(rooms[i].id, "Lab2") == 0) lab2 = rooms[i].type;
    }
    printf("  R101-R103 classroom, Lab1 computer lab, Lab2 electronics lab.\n");
    assert(r101 == ROOM_CLASSROOM && r102 == ROOM_CLASSROOM && r103 == ROOM_CLASSROOM);
    assert(lab1 == ROOM_COMPUTER_LAB);
    assert(lab2 == ROOM_ELECTRONICS_LAB);
    
    Faculty faculties[100]; int num_faculty = 0;
    db_load_faculty(faculties, &num_faculty);
    printf("  Faculty found: %d\n", num_faculty);
    assert(num_faculty == 10);
    
    Division divisions[100]; int num_divisions = 0;
    db_load_divisions(divisions, &num_divisions);
    printf("  Divisions found: %d\n", num_divisions);
    assert(num_divisions == 2);
    
    int divA_size = 0, divB_size = 0;
    for (int i=0; i<num_divisions; i++) {
        if (strcmp(divisions[i].name, "Div A") == 0) divA_size = divisions[i].size;
        if (strcmp(divisions[i].name, "Div B") == 0) divB_size = divisions[i].size;
    }
    printf("  Div A size: %d, Div B size: %d\n", divA_size, divB_size);
    assert(divA_size == 65);
    assert(divB_size == 55);
    
    uint64_t f1=0, f2=0, f4=0, f5=0, f10=0;
    for (int i=0; i<num_faculty; i++) {
        if (strcmp(faculties[i].name, "F1") == 0) f1 = faculties[i].unavail_mask;
        if (strcmp(faculties[i].name, "F2") == 0) f2 = faculties[i].unavail_mask;
        if (strcmp(faculties[i].name, "F4") == 0) f4 = faculties[i].unavail_mask;
        if (strcmp(faculties[i].name, "F5") == 0) f5 = faculties[i].unavail_mask;
        if (strcmp(faculties[i].name, "F10") == 0) f10 = faculties[i].unavail_mask;
    }
    printf("- Masks: F1 %llu; F2 %llu; F4 %llu; F5 %llu; F10 %llu.\n", 
           (unsigned long long)f1, (unsigned long long)f2, (unsigned long long)f4, 
           (unsigned long long)f5, (unsigned long long)f10);
    assert(f1 == 30064771075ULL);
    assert(f2 == 1536ULL);
    assert(f4 == 2080768ULL);
    assert(f5 == 30301501552ULL);
    assert(f10 == 6291456ULL);
    
    Session sessions[100]; int num_sessions = 0;
    db_load_sessions(sessions, &num_sessions);
    
    int div_a_count = 0;
    int div_a_hours = 0;
    int div_b_count = 0;
    int div_b_hours = 0;
    int soft_skills_found = 0;
    int total_hours = 0;
    
    for (int i=0; i<num_sessions; i++) {
        total_hours += sessions[i].duration;
        
        if (strcmp(sessions[i].id, "SOFT") == 0) {
            soft_skills_found++;
            printf("- Soft Skills: student_count %d, splittable %d, fixed_slot %d, joint_divisions={%s, %s}\n", 
                   sessions[i].student_count, sessions[i].splittable, sessions[i].fixed_slot,
                   sessions[i].joint_divisions[0], sessions[i].joint_divisions[1]);
            assert(sessions[i].student_count == 120);
            assert(sessions[i].splittable == 0);
            assert(sessions[i].fixed_slot == 34);
            assert(strcmp(sessions[i].joint_divisions[0], "Div A") == 0);
            assert(strcmp(sessions[i].joint_divisions[1], "Div B") == 0);
        }
        
        // Count Div A
        int in_div_a = 0;
        if (strcmp(sessions[i].division, "Div A") == 0) in_div_a = 1;
        for (int j=0; j<sessions[i].joint_divisions_count; j++) {
            if (strcmp(sessions[i].joint_divisions[j], "Div A") == 0) in_div_a = 1;
        }
        if (in_div_a) {
            div_a_count++;
            div_a_hours += sessions[i].duration;
        }
        
        // Count Div B
        int in_div_b = 0;
        if (strcmp(sessions[i].division, "Div B") == 0) in_div_b = 1;
        for (int j=0; j<sessions[i].joint_divisions_count; j++) {
            if (strcmp(sessions[i].joint_divisions[j], "Div B") == 0) in_div_b = 1;
        }
        if (in_div_b) {
            div_b_count++;
            div_b_hours += sessions[i].duration;
        }
        
        // Room requirements checks
        if (strcmp(sessions[i].faculty, "F6") == 0 || strcmp(sessions[i].faculty, "F7") == 0) {
            if (strstr(sessions[i].id, "LAB") || strstr(sessions[i].subject, "Lab")) {
                assert(sessions[i].required_type == ROOM_COMPUTER_LAB);
            }
        }
        if (strcmp(sessions[i].faculty, "F8") == 0 && (strstr(sessions[i].id, "LAB") || strstr(sessions[i].subject, "Lab"))) {
            assert(sessions[i].required_type == ROOM_ELECTRONICS_LAB);
        }
    }
    
    assert(soft_skills_found == 1);
    
    printf("- Div A: %d sessions, %d hours. Div B: %d sessions, %d hours.\n", div_a_count, div_a_hours, div_b_count, div_b_hours);
    assert(div_a_count == 17);
    assert(div_a_hours == 19);
    assert(div_b_count == 20);
    assert(div_b_hours == 23);
    
    printf("- Distinct sessions = %d (Soft Skills is ONE joint session), hours = %d.\n", num_sessions, total_hours);
    assert(num_sessions == 36);
    assert(total_hours == 41);
    
    printf("- F6 and F7 sessions require Lab1; F8 lab sessions require Lab2. (Verified by required_type)\n");

    db_close();
    printf("test_json_loader PASSED\n");
}

int main(void) {
    assert_adversarial_dataset();
    return 0;
}
