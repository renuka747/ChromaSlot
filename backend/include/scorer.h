/*
 * scorer.h — public API for M5: timetable evaluation.
 *
 * Design
 * ──────
 * Calculates a 0-100 score for a generated timetable based on:
 *   - Gaps (0.4): Penalises free periods between classes for faculty/divisions.
 *   - Balance (0.4): Rewards an even distribution of classes across the 5 days.
 *   - Utilisation (0.2): Measures how many of the 35 college slots are active.
 * 
 * Rejects incomplete timetables immediately.
 */
#ifndef SCORER_H
#define SCORER_H

#include "graph_coloring.h" /* For SessionPlacement */

typedef struct {
    float gap_score;       /* 0 to 100 */
    float balance_score;   /* 0 to 100 */
    float util_score;      /* 0 to 100 */
    float total_score;     /* 0 to 100 */
    int is_complete;       /* 1 if all sessions placed, 0 otherwise */
} TimetableScore;

/*
 * evaluate_timetable()
 * 
 * sessions: Array of sessions
 * num_sessions: Count of sessions
 * placements: Array of placements
 * score_out: Pointer to store the resulting breakdown
 * 
 * Returns 0 if successfully scored. 
 * If incomplete, sets `is_complete = 0`, `total_score = 0` and returns -1.
 */
int evaluate_timetable(const Session *sessions, int num_sessions,
                       const SessionPlacement *placements, 
                       TimetableScore *score_out);

#endif /* SCORER_H */
