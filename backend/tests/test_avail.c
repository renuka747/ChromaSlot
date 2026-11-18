/*
 * test_availability.c — M2 unit tests.
 *
 * Build & run:
 *   cd backend
 *   make test_availability
 *   ./build/test_availability
 *
 * 13 tests covering: init/free, book, double-book guard, unbook no-op,
 * booker tracking, cross-key isolation, slot boundary, avail_slot_free,
 * avail_book_session rollback, avail_unbook_session.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/avail.h"

/* ── helper: build a minimal Session ─────────────────────────────────── */
static void make_session(Session *s, const char *id,
                          const char *faculty, const char *division,
                          const char *room) {
    session_init(s);
    strncpy(s->id,       id,       MAX_ID_LEN   - 1);
    strncpy(s->faculty,  faculty,  MAX_NAME_LEN - 1);
    strncpy(s->division, division, MAX_NAME_LEN - 1);
    strncpy(s->room,     room,     MAX_NAME_LEN - 1);
    strncpy(s->subject,  "Subj",   MAX_NAME_LEN - 1);
}

/* ══════════════════════════════════════════════════════════════════════ */

/* Test 1: avail_init allocates properly; avail_free is safe */
static void test_init_and_free(void) {
    AvailTable t;
    assert(avail_init(&t, 64) == AVAIL_OK);
    assert(t.num_buckets == 64);
    assert(t.entry_count == 0);
    assert(t.buckets     != NULL);
    avail_free(&t);
    assert(t.buckets == NULL);

    /* Second free must not crash */
    avail_free(&t);
    printf("  [PASS] test_init_and_free\n");
}

/* Test 2: basic book → is_booked = 1 */
static void test_book_basic(void) {
    AvailTable t;
    avail_init(&t, 32);

    assert(avail_book(&t, "Dr. Sharma", SLOT(0,0), "CS101-A-1") == AVAIL_OK);
    assert(avail_is_booked(&t, "Dr. Sharma", SLOT(0,0)) == 1);

    avail_free(&t);
    printf("  [PASS] test_book_basic\n");
}

/* Test 3: unbooked slot → is_booked = 0 */
static void test_unbooked_slot_is_free(void) {
    AvailTable t;
    avail_init(&t, 32);

    /* Nothing booked yet */
    assert(avail_is_booked(&t, "Dr. Sharma", SLOT(0,0)) == 0);

    /* Book one slot; a different slot must still be free */
    avail_book(&t, "Dr. Sharma", SLOT(0,0), "CS101-A-1");
    assert(avail_is_booked(&t, "Dr. Sharma", SLOT(0,1)) == 0);

    avail_free(&t);
    printf("  [PASS] test_unbooked_slot_is_free\n");
}

/* Test 4: double-book same key+slot returns AVAIL_ERR_DOUBLE_BOOK */
static void test_double_book_error(void) {
    AvailTable t;
    avail_init(&t, 32);

    assert(avail_book(&t, "R101", SLOT(1,2), "CS101-A-1") == AVAIL_OK);
    /* Trying to book the same slot again must fail, not overwrite */
    int rc = avail_book(&t, "R101", SLOT(1,2), "CS999-X-1");
    assert(rc == AVAIL_ERR_DOUBLE_BOOK);

    /* Original booker must still be intact */
    const char *owner = avail_booker(&t, "R101", SLOT(1,2));
    assert(owner != NULL);
    assert(strcmp(owner, "CS101-A-1") == 0);

    avail_free(&t);
    printf("  [PASS] test_double_book_error\n");
}

/* Test 5: unbook makes the slot free again */
static void test_unbook_releases_slot(void) {
    AvailTable t;
    avail_init(&t, 32);

    avail_book(&t, "D1", SLOT(2,3), "CS101-A-1");
    assert(avail_is_booked(&t, "D1", SLOT(2,3)) == 1);

    avail_unbook(&t, "D1", SLOT(2,3));
    assert(avail_is_booked(&t, "D1", SLOT(2,3)) == 0);

    /* Slot can be rebooked now */
    assert(avail_book(&t, "D1", SLOT(2,3), "CS102-B-1") == AVAIL_OK);

    avail_free(&t);
    printf("  [PASS] test_unbook_releases_slot\n");
}

/* Test 6: unbook on never-booked slot is a silent no-op (no crash) */
static void test_unbook_noop(void) {
    AvailTable t;
    avail_init(&t, 32);

    /* Key doesn't even exist in the table */
    avail_unbook(&t, "Prof. Ghost", SLOT(0,0));

    /* Key exists but slot was never booked */
    avail_book(&t, "Prof. Ghost", SLOT(1,0), "S1");
    avail_unbook(&t, "Prof. Ghost", SLOT(3,3));   /* different slot */

    avail_free(&t);
    printf("  [PASS] test_unbook_noop\n");
}

/* Test 7: avail_booker returns session_id and NULL correctly */
static void test_booker_tracking(void) {
    AvailTable t;
    avail_init(&t, 32);

    avail_book(&t, "Dr. Patil", SLOT(4,6), "ML101-C-1");
    const char *b = avail_booker(&t, "Dr. Patil", SLOT(4,6));
    assert(b != NULL);
    assert(strcmp(b, "ML101-C-1") == 0);

    /* Different slot — no booker */
    assert(avail_booker(&t, "Dr. Patil", SLOT(4,5)) == NULL);

    avail_free(&t);
    printf("  [PASS] test_booker_tracking\n");
}

/* Test 8: different keys do not interfere */
static void test_key_isolation(void) {
    AvailTable t;
    avail_init(&t, 32);

    /* Same slot, different keys — both bookings must succeed */
    assert(avail_book(&t, "Dr. Sharma",   SLOT(0,0), "S1") == AVAIL_OK);
    assert(avail_book(&t, "Prof. Mehta",  SLOT(0,0), "S2") == AVAIL_OK);

    assert(avail_is_booked(&t, "Dr. Sharma",  SLOT(0,0)) == 1);
    assert(avail_is_booked(&t, "Prof. Mehta", SLOT(0,0)) == 1);
    assert(avail_is_booked(&t, "Dr. Kulkarni",SLOT(0,0)) == 0); /* absent key */

    avail_free(&t);
    printf("  [PASS] test_key_isolation\n");
}

/* Test 9: all 35 slots can be booked for one key */
static void test_all_slots_one_key(void) {
    AvailTable t;
    avail_init(&t, 32);

    for (int s = 0; s < NUM_SLOTS; s++) {
        char id[MAX_ID_LEN];
        snprintf(id, sizeof(id), "SES-%02d", s);
        assert(avail_book(&t, "RoomX", s, id) == AVAIL_OK);
    }
    for (int s = 0; s < NUM_SLOTS; s++)
        assert(avail_is_booked(&t, "RoomX", s) == 1);

    avail_free(&t);
    printf("  [PASS] test_all_slots_one_key\n");
}

/* Test 10: invalid slot index returns AVAIL_ERR_INVALID */
static void test_invalid_slot(void) {
    AvailTable t;
    avail_init(&t, 32);

    assert(avail_book(&t, "D1", -1,       "S1") == AVAIL_ERR_INVALID);
    assert(avail_book(&t, "D1", NUM_SLOTS, "S1") == AVAIL_ERR_INVALID);
    assert(avail_is_booked(&t, "D1", -1)       == 0); /* safe, not 1 */
    assert(avail_booker(&t, "D1", NUM_SLOTS)   == NULL);

    avail_free(&t);
    printf("  [PASS] test_invalid_slot\n");
}

/* Test 11: avail_slot_free checks all three resources */
static void test_slot_free_three_resources(void) {
    AvailTracker at;
    avail_tracker_init(&at, 32);

    Session s;
    make_session(&s, "CS101-A-1", "Dr. Sharma", "D1", "R101");

    int slot = SLOT(0, 0);
    assert(avail_slot_free(&at, &s, slot) == 1);  /* all free initially */

    /* Block via faculty table only */
    avail_book(&at.faculty, "Dr. Sharma", slot, "OTHER");
    assert(avail_slot_free(&at, &s, slot) == 0);  /* faculty busy */

    avail_unbook(&at.faculty, "Dr. Sharma", slot);
    assert(avail_slot_free(&at, &s, slot) == 1);

    /* Block via room only */
    avail_book(&at.room, "R101", slot, "OTHER");
    assert(avail_slot_free(&at, &s, slot) == 0);  /* room busy */
    avail_unbook(&at.room, "R101", slot);

    /* Block via division only */
    avail_book(&at.division, "D1", slot, "OTHER");
    assert(avail_slot_free(&at, &s, slot) == 0);  /* division busy */
    avail_unbook(&at.division, "D1", slot);

    avail_tracker_free(&at);
    printf("  [PASS] test_slot_free_three_resources\n");
}

/* Test 12: avail_book_session atomic rollback on room conflict */
static void test_book_session_rollback(void) {
    AvailTracker at;
    avail_tracker_init(&at, 32);

    Session s1, s2;
    /* s1 and s2 share room R101 */
    make_session(&s1, "CS101-A-1", "Dr. Sharma",  "D1", "R101");
    make_session(&s2, "CS102-B-1", "Prof. Mehta", "D2", "R101");

    int slot = SLOT(1, 1);

    /* Book s1 successfully */
    assert(avail_book_session(&at, &s1, slot) == AVAIL_OK);

    /* s2 shares room → must fail; faculty and division of s2 must remain free */
    int rc = avail_book_session(&at, &s2, slot);
    assert(rc == AVAIL_ERR_DOUBLE_BOOK);

    /* s2's faculty and division must NOT have been booked (rollback worked) */
    assert(avail_is_booked(&at.faculty,  "Prof. Mehta", slot) == 0);
    assert(avail_is_booked(&at.division, "D2",          slot) == 0);

    avail_tracker_free(&at);
    printf("  [PASS] test_book_session_rollback\n");
}

/* Test 13: avail_unbook_session releases all three resources */
static void test_unbook_session(void) {
    AvailTracker at;
    avail_tracker_init(&at, 32);

    Session s;
    make_session(&s, "CS101-A-1", "Dr. Sharma", "D1", "R101");
    int slot = SLOT(3, 6);

    avail_book_session(&at, &s, slot);
    assert(avail_slot_free(&at, &s, slot) == 0);

    avail_unbook_session(&at, &s, slot);
    assert(avail_slot_free(&at, &s, slot) == 1);

    /* Calling again on an already-unbooked session must not crash */
    avail_unbook_session(&at, &s, slot);

    avail_tracker_free(&at);
    printf("  [PASS] test_unbook_session\n");
}

/* ── main ─────────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== M2: availability tests ===\n");

    test_init_and_free();
    test_book_basic();
    test_unbooked_slot_is_free();
    test_double_book_error();
    test_unbook_releases_slot();
    test_unbook_noop();
    test_booker_tracking();
    test_key_isolation();
    test_all_slots_one_key();
    test_invalid_slot();
    test_slot_free_three_resources();
    test_book_session_rollback();
    test_unbook_session();

    printf("\nAll M2 tests passed.\n");
    return 0;
}
