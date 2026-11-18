#include "scorer.h"
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

static void test_incomplete_timetable(void) {
    Session sessions[1];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    
    SessionPlacement placements[1];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = -1; // Unplaced
    
    TimetableScore score;
    assert(evaluate_timetable(sessions, 1, placements, &score) == 0);
    assert(score.is_complete == 0); // Correctly marked incomplete
    // When 0 items placed, scores default to 80.0
    assert(score.total_score == 80.0f);
}

static void test_gap_penalty(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F1", "D1", "R2");
    
    SessionPlacement placements[2];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1");
    
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 2; strcpy(placements[1].assigned_room, "R2");
    
    TimetableScore score;
    assert(evaluate_timetable(sessions, 2, placements, &score) == 0);
    
    assert(score.gap_score == 96.0f);
    assert(score.balance_score == 90.0f);
    assert(score.util_score > 5.0f && score.util_score < 6.0f);
}

static void test_no_gaps(void) {
    Session sessions[2];
    make_session(&sessions[0], "S1", "F1", "D1", "R1");
    make_session(&sessions[1], "S2", "F1", "D1", "R2");
    
    SessionPlacement placements[2];
    strcpy(placements[0].session_id, "S1");
    placements[0].assigned_slot = 0; strcpy(placements[0].assigned_room, "R1");
    
    strcpy(placements[1].session_id, "S2");
    placements[1].assigned_slot = 1; strcpy(placements[1].assigned_room, "R2");
    
    TimetableScore score;
    assert(evaluate_timetable(sessions, 2, placements, &score) == 0);
    assert(score.gap_score == 100.0f); // 0 gaps
}

int main(void) {
    printf("=== M5: scorer tests ===\n");
    test_incomplete_timetable();
    printf("  [PASS] test_incomplete_timetable\n");
    test_gap_penalty();
    printf("  [PASS] test_gap_penalty\n");
    test_no_gaps();
    printf("  [PASS] test_no_gaps\n");
    
    printf("\nAll M5 tests passed.\n");
    return 0;
}
