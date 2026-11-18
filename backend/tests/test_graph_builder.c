/*
 * test_graph_builder.c — M1 unit tests.
 *
 * Hand-crafted data; no external testing framework needed.
 * Build & run:
 *   cd backend
 *   make test_graph_builder
 *   ./test_graph_builder
 *
 * All tests use assert(); a failing assert prints the line number and aborts.
 * A clean run prints "All M1 tests passed." and exits 0.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../include/graph_builder.h"

/* ── helpers ─────────────────────────────────────────────────────────── */

/* Build a minimal session quickly */
static void make_session(Session *s,
                          const char *id,
                          const char *faculty,
                          const char *division,
                          const char *room) {
    session_init(s);
    strncpy(s->id,       id,       MAX_ID_LEN   - 1);
    strncpy(s->faculty,  faculty,  MAX_NAME_LEN - 1);
    strncpy(s->division, division, MAX_NAME_LEN - 1);
    strncpy(s->room,     room,     MAX_NAME_LEN - 1);
    strncpy(s->subject,  "SubjectX", MAX_NAME_LEN - 1);
}

/* ── Test 1: session_init ─────────────────────────────────────────────── */
static void test_session_init(void) {
    Session s;
    /* Deliberately dirty the memory first */
    memset(&s, 0xFF, sizeof(s));
    session_init(&s);

    assert(s.fixed_slot    == -1);
    assert(s.unavail_count == 0);
    assert(s.id[0]         == '\0');
    assert(s.faculty[0]    == '\0');
    printf("  [PASS] test_session_init\n");
}

/* ── Test 2: conflicts() — no conflict ────────────────────────────────── */
static void test_conflicts_none(void) {
    Session a, b;
    make_session(&a, "CS101-A-1", "Dr. Sharma",   "D1", "R101");
    make_session(&b, "CS102-B-2", "Prof. Mehta",  "D2", "R202");

    ConflictReason r = conflicts(&a, &b);
    assert(r == 0);
    printf("  [PASS] test_conflicts_none\n");
}

/* ── Test 3: conflicts() — faculty only ──────────────────────────────── */
static void test_conflicts_faculty(void) {
    Session a, b;
    make_session(&a, "CS101-A-1", "Dr. Sharma", "D1", "R101");
    make_session(&b, "CS103-B-1", "Dr. Sharma", "D2", "R202");  /* same faculty */

    ConflictReason r = conflicts(&a, &b);
    assert(r & CONFLICT_FACULTY);
    assert(!(r & CONFLICT_ROOM));
    assert(!(r & CONFLICT_DIVISION));
    printf("  [PASS] test_conflicts_faculty\n");
}

/* ── Test 4: conflicts() — room only ─────────────────────────────────── */
static void test_conflicts_room(void) {
    Session a, b;
    make_session(&a, "CS101-A-1", "Dr. Sharma",  "D1", "R101");
    make_session(&b, "MA101-B-1", "Dr. Deshmukh","D2", "R101");  /* same room */

    ConflictReason r = conflicts(&a, &b);
    assert(!(r & CONFLICT_FACULTY));
    assert(r & CONFLICT_ROOM);
    assert(!(r & CONFLICT_DIVISION));
    printf("  [PASS] test_conflicts_room\n");
}

/* ── Test 5: conflicts() — division only ─────────────────────────────── */
static void test_conflicts_division(void) {
    Session a, b;
    make_session(&a, "CS101-A-1", "Dr. Sharma", "D1", "R101");
    make_session(&b, "CS104-A-2", "Prof. Mehta","D1", "R202");  /* same division */

    ConflictReason r = conflicts(&a, &b);
    assert(!(r & CONFLICT_FACULTY));
    assert(!(r & CONFLICT_ROOM));
    assert(r & CONFLICT_DIVISION);
    printf("  [PASS] test_conflicts_division\n");
}

/* ── Test 6: conflicts() — all three ─────────────────────────────────── */
static void test_conflicts_all(void) {
    Session a, b;
    /* Identical faculty + room + division → all three reasons */
    make_session(&a, "CS101-A-1", "Dr. Sharma", "D1", "R101");
    make_session(&b, "CS101-A-2", "Dr. Sharma", "D1", "R101");

    ConflictReason r = conflicts(&a, &b);
    assert(r & CONFLICT_FACULTY);
    assert(r & CONFLICT_ROOM);
    assert(r & CONFLICT_DIVISION);
    printf("  [PASS] test_conflicts_all\n");
}

/* ── Test 7: graph_build — 4 sessions, known topology ────────────────── *
 *                                                                         *
 *  Sessions:                                                              *
 *    s0: faculty=A, div=D1, room=R1  ─┐                                  *
 *    s1: faculty=A, div=D2, room=R2  ─┘ share faculty → edge 0-1         *
 *    s2: faculty=B, div=D1, room=R3     shares div with s0 → edge 0-2    *
 *    s3: faculty=C, div=D3, room=R4     no overlap with any              *
 *                                                                         *
 *  Expected degrees: s0=2, s1=1, s2=1, s3=0                              */
static void test_graph_build_topology(void) {
    Session sessions[4];
    make_session(&sessions[0], "S0", "FacA", "D1", "R1");
    make_session(&sessions[1], "S1", "FacA", "D2", "R2");
    make_session(&sessions[2], "S2", "FacB", "D1", "R3");
    make_session(&sessions[3], "S3", "FacC", "D3", "R4");

    ConflictGraph g;
    int rc = graph_build(&g, sessions, 4);
    assert(rc == 0);

    assert(graph_degree(&g, 0) == 2);  /* edges to s1 (faculty) and s2 (division) */
    assert(graph_degree(&g, 1) == 1);  /* edge to s0 (faculty)                    */
    assert(graph_degree(&g, 2) == 1);  /* edge to s0 (division)                   */
    assert(graph_degree(&g, 3) == 0);  /* no conflicts                            */

    /* Verify edge 0→1 has CONFLICT_FACULTY */
    AdjNode *node = g.adj[0];
    int found_faculty = 0, found_division = 0;
    while (node) {
        if (node->neighbour_idx == 1 && (node->reasons & CONFLICT_FACULTY))
            found_faculty = 1;
        if (node->neighbour_idx == 2 && (node->reasons & CONFLICT_DIVISION))
            found_division = 1;
        node = node->next;
    }
    assert(found_faculty  == 1);
    assert(found_division == 1);

    graph_free(&g);
    printf("  [PASS] test_graph_build_topology\n");
}

/* ── Test 8: graph_free is safe to call on an empty graph ─────────────── */
static void test_graph_free_empty(void) {
    ConflictGraph g;
    memset(&g, 0, sizeof(g));
    graph_free(&g);   /* must not crash */
    printf("  [PASS] test_graph_free_empty\n");
}

/* ── Test 9: conflict_reason_str ─────────────────────────────────────── */
static void test_conflict_reason_str(void) {
    char buf[MAX_REASON_LEN];

    conflict_reason_str(0, buf, sizeof(buf));
    assert(strcmp(buf, "None") == 0);

    conflict_reason_str(CONFLICT_FACULTY, buf, sizeof(buf));
    assert(strcmp(buf, "Faculty") == 0);

    conflict_reason_str(CONFLICT_FACULTY | CONFLICT_ROOM, buf, sizeof(buf));
    assert(strcmp(buf, "Faculty, Room") == 0);

    conflict_reason_str(CONFLICT_FACULTY | CONFLICT_ROOM | CONFLICT_DIVISION,
                        buf, sizeof(buf));
    assert(strcmp(buf, "Faculty, Room, Division") == 0);

    printf("  [PASS] test_conflict_reason_str\n");
}

/* ── Test 10: SLOT macros in constants.h ─────────────────────────────── */
static void test_slot_macros(void) {
    /* SLOT(day, period) round-trips through SLOT_DAY / SLOT_PERIOD */
    for (int d = 0; d < NUM_DAYS; d++) {
        for (int p = 0; p < NUM_PERIODS; p++) {
            int s = SLOT(d, p);
            assert(SLOT_DAY(s)    == d);
            assert(SLOT_PERIOD(s) == p);
            assert(SLOT_VALID(s));
        }
    }
    assert(!SLOT_VALID(-1));
    assert(!SLOT_VALID(NUM_SLOTS));
    assert(NUM_SLOTS == NUM_DAYS * NUM_PERIODS);
    printf("  [PASS] test_slot_macros\n");
}

/* ── main ────────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== M1: graph_builder tests ===\n");

    test_session_init();
    test_conflicts_none();
    test_conflicts_faculty();
    test_conflicts_room();
    test_conflicts_division();
    test_conflicts_all();
    test_graph_build_topology();
    test_graph_free_empty();
    test_conflict_reason_str();
    test_slot_macros();

    printf("\nAll M1 tests passed.\n");
    return 0;
}
