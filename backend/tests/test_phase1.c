#include "core_types.h"
#include "db.h"
#include "hash_map.h"
#include "json_loader.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "graph_builder.h"

void test_hash_map(void) {
    HashMap map;
    assert(hash_map_init(&map, 10) == 0);
    
    int val1 = 42;
    int val2 = 99;
    
    assert(hash_map_put(&map, "key1", &val1) == 0);
    assert(hash_map_put(&map, "key2", &val2) == 0);
    
    int *ret1 = (int*)hash_map_get(&map, "key1");
    int *ret2 = (int*)hash_map_get(&map, "key2");
    
    assert(ret1 && *ret1 == 42);
    assert(ret2 && *ret2 == 99);
    
    hash_map_free(&map);
    printf("test_hash_map PASSED\n");
}

void assert_session_eq(const Session *a, const Session *b) {
    assert(strcmp(a->id, b->id) == 0);
    assert(strcmp(a->subject, b->subject) == 0);
    assert(strcmp(a->faculty, b->faculty) == 0);
    assert(strcmp(a->division, b->division) == 0);
    assert(strcmp(a->room, b->room) == 0);
    assert(a->fixed_slot == b->fixed_slot);
    assert(a->unavail_count == b->unavail_count);
    for(int i=0; i<a->unavail_count; i++) assert(a->unavail[i] == b->unavail[i]);
    assert(a->student_count == b->student_count);
    assert(a->duration == b->duration);
    assert(a->allowed_rooms_count == b->allowed_rooms_count);
    for(int i=0; i<a->allowed_rooms_count; i++) assert(strcmp(a->allowed_rooms[i], b->allowed_rooms[i]) == 0);
    assert(a->joint_divisions_count == b->joint_divisions_count);
    for(int i=0; i<a->joint_divisions_count; i++) assert(strcmp(a->joint_divisions[i], b->joint_divisions[i]) == 0);
    assert(a->required_type == b->required_type);
    assert(strcmp(a->parent_id, b->parent_id) == 0);
    assert(a->batch_index == b->batch_index);
    assert(a->splittable == b->splittable);
}

void assert_room_eq(const Room *a, const Room *b) {
    assert(strcmp(a->id, b->id) == 0);
    assert(a->capacity == b->capacity);
    assert(a->type == b->type);
}

void assert_faculty_eq(const Faculty *a, const Faculty *b) {
    assert(strcmp(a->name, b->name) == 0);
    assert(a->max_consecutive == b->max_consecutive);
    assert(a->unavail_mask == b->unavail_mask);
}

void assert_division_eq(const Division *a, const Division *b) {
    assert(strcmp(a->name, b->name) == 0);
    assert(a->size == b->size);
}

void test_db_migration(void) {
    unlink("test_phase1.db");
    assert(db_init("test_phase1.db") == 0);
    
    Session s;
    session_init(&s);
    strcpy(s.id, "TEST-1");
    strcpy(s.subject, "Math");
    s.student_count = 55;
    s.duration = 2;
    strcpy(s.allowed_rooms[0], "R1");
    s.allowed_rooms_count = 1;
    strcpy(s.joint_divisions[0], "D1");
    strcpy(s.joint_divisions[1], "D2");
    s.joint_divisions_count = 2;
    s.required_type = ROOM_COMPUTER_LAB;
    strcpy(s.parent_id, "TEST-PARENT");
    s.batch_index = 1;
    s.splittable = 1;
    s.unavail[0] = 5;
    s.unavail[1] = 10;
    s.unavail_count = 2;
    
    assert(db_save_session(&s) == 0);
    Session loaded[5]; int num = 0;
    assert(db_load_sessions(loaded, &num) == 0);
    assert(num == 1);
    assert_session_eq(&s, &loaded[0]);
    
    Room r = {"R1", 100, ROOM_CLASSROOM};
    assert(db_save_room(&r) == 0);
    Room r_loaded[5]; int r_num = 0;
    assert(db_load_rooms(r_loaded, &r_num) == 0);
    assert(r_num == 1);
    assert_room_eq(&r, &r_loaded[0]);
    
    Faculty f = {"Dr. Smith", 3, 12345ULL};
    assert(db_save_faculty(&f) == 0);
    Faculty f_loaded[5]; int f_num = 0;
    assert(db_load_faculty(f_loaded, &f_num) == 0);
    assert(f_num == 1);
    assert_faculty_eq(&f, &f_loaded[0]);
    
    Division d = {"D1", 60};
    assert(db_save_division(&d) == 0);
    Division d_loaded[5]; int d_num = 0;
    assert(db_load_divisions(d_loaded, &d_num) == 0);
    assert(d_num == 1);
    assert_division_eq(&d, &d_loaded[0]);

    db_close();
    printf("test_db_migration PASSED\n");
}

int main(void) {
    test_hash_map();
    test_db_migration();
    printf("\nTested functions:\n - test_hash_map\n - test_db_migration\n - assert_session_eq (tests id, subject, faculty, division, room, fixed_slot, unavail_count, unavail array, student_count, duration, allowed_rooms_count, allowed_rooms array, joint_divisions_count, joint_divisions array, required_type, parent_id, batch_index, splittable)\n - assert_room_eq\n - assert_faculty_eq\n - assert_division_eq\n\nALL PHASE 1 TESTS PASSED\n");
    return 0;
}
