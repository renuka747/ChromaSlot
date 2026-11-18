/*
 * availability.c — M2 implementation.
 *
 * Hash function: djb2
 *   hash = 5381
 *   for each character c:  hash = hash * 33 ^ c
 * Good distribution for short strings; no stdlib dependency.
 *
 * All memory that is allocated here is freed in avail_free().
 */
#include "avail.h"

#include <stdlib.h>   /* malloc, calloc, free           */
#include <string.h>   /* strcmp, strncpy, memset, strlen */

/* ── djb2 hash ───────────────────────────────────────────────────────── *
 * Map a string key to a bucket index in [0, num_buckets).                *
 * Time: O(|key|)  — practically O(1) for the short names we use         */
static unsigned int djb2(const char *key, int num_buckets) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*key++) != 0)
        hash = ((hash << 5) + hash) ^ (unsigned long)c;   /* hash*33 ^ c */
    return (unsigned int)(hash % (unsigned long)num_buckets);
}

/* ── find_entry (internal) ───────────────────────────────────────────── *
 * Return the AvailEntry for `key`, or NULL if not found.                 *
 * Time: O(chain length) ≈ O(1) for low load factor                      */
static AvailEntry *find_entry(const AvailTable *t, const char *key) {
    unsigned int bucket = djb2(key, t->num_buckets);
    AvailEntry *e = t->buckets[bucket];
    while (e) {
        if (strcmp(e->key, key) == 0) return e;
        e = e->next;
    }
    return NULL;
}

/* ── find_or_create_entry (internal) ────────────────────────────────── *
 * Return the existing entry for `key`, or allocate a new one and        *
 * prepend it to the bucket chain.                                        *
 * Returns NULL only on malloc failure.                                   *
 * Time: O(1) amortised                                                   */
static AvailEntry *find_or_create_entry(AvailTable *t, const char *key) {
    AvailEntry *e = find_entry(t, key);
    if (e) return e;

    /* Allocate new entry */
    e = (AvailEntry *)malloc(sizeof(AvailEntry));
    if (!e) return NULL;

    memset(e, 0, sizeof(AvailEntry));
    strncpy(e->key, key, MAX_NAME_LEN - 1);

    /* Prepend to bucket chain */
    unsigned int bucket = djb2(key, t->num_buckets);
    e->next             = t->buckets[bucket];
    t->buckets[bucket]  = e;
    t->entry_count++;

    return e;
}

/* ═══════════════════════════════════════════════════════════════════════ *
 *  AvailTable — low-level implementation                                  *
 * ═══════════════════════════════════════════════════════════════════════ */

/* avail_init — allocate the bucket array.
 * Time: O(num_buckets) */
int avail_init(AvailTable *t, int num_buckets) {
    if (!t || num_buckets <= 0) return AVAIL_ERR_INVALID;
    t->num_buckets = num_buckets;
    t->entry_count = 0;
    t->buckets = (AvailEntry **)calloc(num_buckets, sizeof(AvailEntry *));
    return t->buckets ? AVAIL_OK : AVAIL_ERR_ALLOC;
}

/* avail_free — walk every chain, free every node, free bucket array.
 * Time: O(num_buckets + entry_count) */
void avail_free(AvailTable *t) {
    if (!t || !t->buckets) return;
    for (int i = 0; i < t->num_buckets; i++) {
        AvailEntry *cur = t->buckets[i];
        while (cur) {
            AvailEntry *nxt = cur->next;
            free(cur);
            cur = nxt;
        }
    }
    free(t->buckets);
    t->buckets     = NULL;
    t->num_buckets = 0;
    t->entry_count = 0;
}

/* avail_book — mark slot as booked; fail on double-booking.
 * Time: O(1) amortised */
int avail_book(AvailTable *t, const char *key, int slot,
               const char *session_id) {
    /* Validate inputs */
    if (!t || !key || !session_id) return AVAIL_ERR_INVALID;
    if (!SLOT_VALID(slot))         return AVAIL_ERR_INVALID;

    AvailEntry *e = find_or_create_entry(t, key);
    if (!e) return AVAIL_ERR_ALLOC;

    uint64_t mask = (uint64_t)1 << slot;

    /* Check for double-booking BEFORE writing anything */
    if (e->booked_mask & mask) return AVAIL_ERR_DOUBLE_BOOK;

    /* Commit the booking */
    e->booked_mask |= mask;
    strncpy(e->booker[slot], session_id, MAX_ID_LEN - 1);

    return AVAIL_OK;
}

/* avail_unbook — clear a booking; silent no-op if slot was free.
 * Time: O(1) amortised */
void avail_unbook(AvailTable *t, const char *key, int slot) {
    if (!t || !key || !SLOT_VALID(slot)) return;

    AvailEntry *e = find_entry(t, key);
    if (!e) return;   /* key never existed — safe no-op */

    e->booked_mask &= ~((uint64_t)1 << slot);
    e->booker[slot][0] = '\0';  /* clear the booker name */
}

/* avail_is_booked — 1 if the slot is occupied, 0 otherwise.
 * Time: O(1) amortised */
int avail_is_booked(const AvailTable *t, const char *key, int slot) {
    if (!t || !key || !SLOT_VALID(slot)) return 0;

    const AvailEntry *e = find_entry(t, key);
    if (!e) return 0;

    return (e->booked_mask >> slot) & 1;
}

/* avail_booker — return session_id that owns the slot, or NULL if free.
 * Time: O(1) amortised */
const char *avail_booker(const AvailTable *t, const char *key, int slot) {
    if (!t || !key || !SLOT_VALID(slot)) return NULL;

    const AvailEntry *e = find_entry(t, key);
    if (!e) return NULL;

    if (!((e->booked_mask >> slot) & 1)) return NULL;  /* slot is free */
    return e->booker[slot];
}

/* ═══════════════════════════════════════════════════════════════════════ *
 *  AvailTracker — high-level implementation                               *
 * ═══════════════════════════════════════════════════════════════════════ */

/* avail_tracker_init — initialise faculty, room, division tables.
 * Time: O(num_buckets) */
int avail_tracker_init(AvailTracker *at, int num_buckets) {
    if (!at) return AVAIL_ERR_INVALID;
    memset(at, 0, sizeof(AvailTracker));

    int rc;
    rc = avail_init(&at->faculty,  num_buckets); if (rc) goto fail;
    rc = avail_init(&at->room,     num_buckets); if (rc) goto fail;
    rc = avail_init(&at->division, num_buckets); if (rc) goto fail;
    return AVAIL_OK;

fail:
    avail_tracker_free(at);
    return rc;
}

/* avail_tracker_free — free all three tables.
 * Time: O(B + E) */
void avail_tracker_free(AvailTracker *at) {
    if (!at) return;
    avail_free(&at->faculty);
    avail_free(&at->room);
    avail_free(&at->division);
}

/* avail_slot_free — 1 if faculty AND room AND division are all free.
 * Time: O(1) */
int avail_slot_free(const AvailTracker *at, const Session *s, int slot) {
    if (!at || !s || !SLOT_VALID(slot)) return 0;
    return !avail_is_booked(&at->faculty,  s->faculty,  slot) &&
           !avail_is_booked(&at->room,     s->room,     slot) &&
           !avail_is_booked(&at->division, s->division, slot);
}

/* avail_book_session — atomically book faculty + room + division.
 * If any resource is already taken, the successful bookings are rolled
 * back and the table is left unchanged.
 * Time: O(1) */
int avail_book_session(AvailTracker *at, const Session *s, int slot) {
    if (!at || !s || !SLOT_VALID(slot)) return AVAIL_ERR_INVALID;

    /* Try to book faculty first */
    int rc = avail_book(&at->faculty, s->faculty, slot, s->id);
    if (rc != AVAIL_OK) return rc;

    /* Try to book room */
    rc = avail_book(&at->room, s->room, slot, s->id);
    if (rc != AVAIL_OK) {
        avail_unbook(&at->faculty, s->faculty, slot);  /* roll back */
        return rc;
    }

    /* Try to book division */
    rc = avail_book(&at->division, s->division, slot, s->id);
    if (rc != AVAIL_OK) {
        avail_unbook(&at->faculty, s->faculty, slot);  /* roll back */
        avail_unbook(&at->room,    s->room,    slot);  /* roll back */
        return rc;
    }

    return AVAIL_OK;
}

/* avail_unbook_session — release faculty + room + division.
 * All three unbooks are always attempted (all are safe no-ops).
 * Time: O(1) */
void avail_unbook_session(AvailTracker *at, const Session *s, int slot) {
    if (!at || !s || !SLOT_VALID(slot)) return;
    avail_unbook(&at->faculty,  s->faculty,  slot);
    avail_unbook(&at->room,     s->room,     slot);
    avail_unbook(&at->division, s->division, slot);
}
