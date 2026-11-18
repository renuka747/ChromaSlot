/*
 * preflight.h — Phase 3 preflight checks and auto batch split
 */
#ifndef PREFLIGHT_H
#define PREFLIGHT_H

#include "core_types.h"
#include "hash_map.h"

/*
 * fit_room() - Checks if `room` can accommodate `session`.
 * Matches required_type, capacity >= student_count, and allowed_rooms filter.
 * Returns 1 if it fits, 0 otherwise.
 * Time: O(A) where A is the number of allowed rooms (max 5).
 */
int fit_room(const Session *session, const Room *room);

/*
 * split_into_batches() - Splits splittable sessions exceeding max allowed room capacity.
 * Mutates `sessions` array and increments `*num_sessions`.
 * Time: O(S * R) where S is num_sessions and R is num_rooms.
 */
int split_into_batches(Session *sessions, int *num_sessions, const Room *rooms, int num_rooms);

/*
 * preflight_check() - Checks all sessions for intrinsic impossibility.
 * Populates `placements` with REASON_CAPACITY, REASON_FIXED_SLOT_CLASH, 
 * or REASON_FACULTY_UNAVAIL as applicable.
 * Returns the number of impossible sessions found.
 * Time: O(S * R)
 */
int preflight_check(const Session *sessions, int num_sessions, const Room *rooms, int num_rooms, const HashMap *faculty_map, SessionPlacement *placements);

#endif /* PREFLIGHT_H */
