/*
 * slot_helpers.h — Phase 2 atomic placement and slot helpers
 */
#ifndef SLOT_HELPERS_H
#define SLOT_HELPERS_H

#include "core_types.h"
#include "avail.h"
#include "hash_map.h"

/*
 * valid_start() - Checks if a block of `duration` starting at `slot` is valid.
 * Valid blocks stay on the same day and never cross lunch (periods 3 and 4).
 * Returns 1 if valid, 0 otherwise.
 * Time: O(1)
 */
int valid_start(int slot, int duration);

/*
 * placement_slots() - Populates `out_slots` with the slots occupied by `placement`.
 * Returns the number of slots occupied (which is session->duration), or 0 if unplaced.
 * Time: O(D) where D = duration.
 */
int placement_slots(const SessionPlacement *placement, const Session *session, int out_slots[]);

/*
 * try_place() - Atomically attempts to book a session at `start_slot` in `room`.
 * Checks faculty mask, double-booking for faculty/room/divisions, and consecutive cap.
 * If any check fails, leaves the tracker completely unmodified.
 * Returns REASON_NONE on success, or the specific UnplacedReason on failure.
 * Time: O(D * V) where D = duration, V = number of divisions.
 */
UnplacedReason try_place(AvailTracker *tracker, const HashMap *faculty_map, const Session *session, int start_slot, const char *room);

/*
 * unplace() - Exact inverse of try_place(). Removes the session from all trackers.
 * Time: O(D * V).
 */
void unplace(AvailTracker *tracker, const Session *session, int start_slot, const char *room);

#endif /* SLOT_HELPERS_H */
