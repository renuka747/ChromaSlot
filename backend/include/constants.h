/*
 * constants.h — single source of truth for timetable dimensions.
 * Every module must include this file; never hard-code 5 or 7 elsewhere.
 */
#ifndef CONSTANTS_H
#define CONSTANTS_H

/* ── Timetable dimensions ─────────────────────────────────────────────── */
#define NUM_DAYS     5   /* Mon=0 … Fri=4                                  */
#define NUM_PERIODS  7   /* 0=09:00, 1=10:00, … 3=12:00, 4=14:00, … 6=16:00 */
#define NUM_SLOTS    35  /* NUM_DAYS * NUM_PERIODS                         */

/* Period 3 ends at 13:00; lunch occupies 13:00-14:00 (not a usable slot).
 * Period 4 therefore starts at 14:00.  No slot index maps to 13:xx.       */
#define LUNCH_AFTER_PERIOD 3   /* last period before lunch break           */

/* Day labels (index → string) */
static const char * const DAY_NAMES[NUM_DAYS] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday"
};

/* Period start times as human-readable strings (index → HH:MM) */
static const char * const PERIOD_START[NUM_PERIODS] = {
    "09:00", "10:00", "11:00", "12:00", "14:00", "15:00", "16:00"
};

/* Period end times */
static const char * const PERIOD_END[NUM_PERIODS] = {
    "10:00", "11:00", "12:00", "13:00", "15:00", "16:00", "17:00"
};

/* ── Slot encoding ────────────────────────────────────────────────────── *
 * A TimeSlot is stored as a single int: slot = day * NUM_PERIODS + period  *
 * Use the macros below for all conversions.                                */
#define SLOT(day, period)       ((day) * NUM_PERIODS + (period))
#define SLOT_DAY(slot)          ((slot) / NUM_PERIODS)
#define SLOT_PERIOD(slot)       ((slot) % NUM_PERIODS)
#define SLOT_VALID(slot)        ((slot) >= 0 && (slot) < NUM_SLOTS)

/* ── String size limits ──────────────────────────────────────────────── */
#define MAX_ID_LEN       64   /* session id, e.g. "CS101-A-1"             */
#define MAX_NAME_LEN    128   /* subject, faculty, division, room name     */
#define MAX_REASON_LEN  256   /* conflict / error reason string            */

/* ── Capacity limits (static upper bounds for this implementation) ────── */
#define MAX_SESSIONS     512  /* maximum sessions in one timetable         */
#define MAX_UNAVAIL      35   /* max unavailable slots per session/faculty */

#endif /* CONSTANTS_H */
