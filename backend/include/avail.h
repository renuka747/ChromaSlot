/*
 * availability.h — public API for M2: hash-based booked-slot tracking.
 *
 * Design
 * ──────
 * One AvailTable is a hash table: string key → set of booked slots.
 *   key   = faculty name, room name, or division name (exact string match)
 *   value = bitmask (uint64_t, bits 0–34 map to slots 0–34) + who booked each
 *
 * Three AvailTables are wrapped by AvailTracker (faculty / room / division).
 * The graph-coloring layer (M3) holds one AvailTracker and calls
 *   avail_book_session / avail_unbook_session / avail_slot_free.
 *
 * Collision resolution: separate chaining (linked list per bucket).
 * Hash function: djb2 (simple, fast, good distribution for short strings).
 *
 * Error codes (AVAIL_OK = 0; any non-zero = failure)
 */
#ifndef AVAILABILITY_H
#define AVAILABILITY_H

#include "constants.h"
#include "graph_builder.h"  /* Session struct */
#include <stdint.h>         /* uint64_t       */
#include <stddef.h>         /* size_t         */

/* ── Error codes ─────────────────────────────────────────────────────── */
#define AVAIL_OK              0
#define AVAIL_ERR_DOUBLE_BOOK 1   /* slot already occupied                */
#define AVAIL_ERR_INVALID     2   /* bad slot index or NULL pointer        */
#define AVAIL_ERR_ALLOC       3   /* malloc returned NULL                  */

/* ── AvailEntry ───────────────────────────────────────────────────────── *
 * One entry (bucket chain node) in the hash table.                        *
 * booked_mask: bit i is set iff slot i is currently booked.               *
 * booker[i]:   session id that owns slot i (empty string if unbooked).    */
typedef struct AvailEntry {
    char            key[MAX_NAME_LEN];          /* faculty / room / div   */
    uint64_t        booked_mask;                /* bitmask of 35 slots    */
    char            booker[NUM_SLOTS][MAX_ID_LEN]; /* session_id per slot */
    struct AvailEntry *next;                    /* collision chain        */
} AvailEntry;

/* ── AvailTable ───────────────────────────────────────────────────────── *
 * Generic hash table: string → booked slot set.                           *
 * Create one per resource class (faculty, room, division).                */
typedef struct {
    AvailEntry **buckets;     /* array of `num_buckets` chain heads        */
    int          num_buckets; /* number of hash buckets (use a prime)      */
    int          entry_count; /* number of distinct keys stored            */
} AvailTable;

/* ── AvailTracker ────────────────────────────────────────────────────── *
 * Wraps three AvailTables.  M3 creates exactly one AvailTracker per      *
 * timetable generation run.                                               */
typedef struct {
    AvailTable faculty;
    AvailTable room;
    AvailTable division;
} AvailTracker;

/* ═══════════════════════════════════════════════════════════════════════ *
 *  AvailTable — low-level API                                             *
 * ═══════════════════════════════════════════════════════════════════════ */

/*
 * avail_init() — initialise an AvailTable with `num_buckets` buckets.
 * Caller must call avail_free() when done.
 * Returns AVAIL_OK or AVAIL_ERR_ALLOC.
 * Time: O(num_buckets)
 */
int avail_init(AvailTable *t, int num_buckets);

/*
 * avail_free() — release all heap memory owned by the table.
 * Safe to call on a zero-initialised table.
 * Time: O(num_buckets + entry_count)
 */
void avail_free(AvailTable *t);

/*
 * avail_book() — mark `slot` as booked by `session_id` under `key`.
 * Returns AVAIL_ERR_DOUBLE_BOOK if already occupied (never overwrites).
 * Returns AVAIL_ERR_INVALID if slot is out of range or pointers are NULL.
 * Returns AVAIL_ERR_ALLOC if a new entry could not be allocated.
 * Returns AVAIL_OK on success.
 * Time: O(1) amortised (hash + possible chain traversal)
 */
int avail_book(AvailTable *t, const char *key, int slot,
               const char *session_id);

/*
 * avail_unbook() — clear the booking for `slot` under `key`.
 * Safe no-op if the slot was never booked or the key doesn't exist.
 * Time: O(1) amortised
 */
void avail_unbook(AvailTable *t, const char *key, int slot);

/*
 * avail_is_booked() — return 1 if `slot` is booked under `key`, else 0.
 * Also returns 0 for invalid inputs (no silent failures from callers).
 * Time: O(1) amortised
 */
int avail_is_booked(const AvailTable *t, const char *key, int slot);

/*
 * avail_booker() — return the session_id that booked `slot` under `key`.
 * Returns NULL if the slot is free, the key is unknown, or input is invalid.
 * Time: O(1) amortised
 */
const char *avail_booker(const AvailTable *t, const char *key, int slot);

/* ═══════════════════════════════════════════════════════════════════════ *
 *  AvailTracker — high-level API (used by M3 graph colouring)            *
 * ═══════════════════════════════════════════════════════════════════════ */

/*
 * avail_tracker_init() — initialise all three inner tables.
 * Returns AVAIL_OK or AVAIL_ERR_ALLOC.
 * Time: O(num_buckets)
 */
int avail_tracker_init(AvailTracker *at, int num_buckets);

/*
 * avail_tracker_free() — free all three tables.
 * Time: O(B + E) where B = buckets, E = entries
 */
void avail_tracker_free(AvailTracker *at);

/*
 * avail_slot_free() — return 1 if slot is free for ALL THREE resources
 *   (faculty, room, division) needed by session `s`.
 * Returns 0 if any resource is occupied.
 * Time: O(1)
 */
int avail_slot_free(const AvailTracker *at, const Session *s, int slot);

/*
 * avail_book_session() — atomically book a slot for faculty, room AND division.
 * If any one is already taken, rolls back the others and returns the
 * appropriate AVAIL_ERR_* code, leaving the table unchanged.
 * Time: O(1)
 */
int avail_book_session(AvailTracker *at, const Session *s, int slot);

/*
 * avail_unbook_session() — release the slot from faculty, room AND division.
 * Safe no-op if the session was never booked there.
 * Time: O(1)
 */
void avail_unbook_session(AvailTracker *at, const Session *s, int slot);

#endif /* AVAILABILITY_H */
