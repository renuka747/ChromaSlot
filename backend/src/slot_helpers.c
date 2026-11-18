#include "slot_helpers.h"
#include <string.h>
#include <stdio.h>

int valid_start(int slot, int duration) {
    if (slot < 0 || slot >= NUM_SLOTS || duration <= 0) return 0;
    int end_slot = slot + duration - 1;
    if (end_slot >= NUM_SLOTS) return 0;
    if (slot / 7 != end_slot / 7) return 0; 
    int p_start = slot % 7;
    int p_end = end_slot % 7;
    if (p_start <= 3 && p_end <= 3) return 1;
    if (p_start >= 4 && p_end >= 4) return 1;
    return 0; 
}

int placement_slots(const SessionPlacement *placement, const Session *session, int out_slots[]) {
    if (!placement || !session || !out_slots) return 0;
    if (placement->assigned_slot < 0) return 0;
    for (int i = 0; i < session->duration; i++) {
        out_slots[i] = placement->assigned_slot + i;
    }
    return session->duration;
}

static int are_consecutive(int s1, int s2) {
    if (s1 > s2) { int tmp = s1; s1 = s2; s2 = tmp; }
    if (s2 != s1 + 1) return 0;
    if (s1 / 7 != s2 / 7) return 0; 
    if (s1 % 7 == 3 && s2 % 7 == 4) return 0; 
    return 1;
}

UnplacedReason try_place(AvailTracker *tracker, const HashMap *faculty_map, const Session *session, int start_slot, const char *room) {
    if (!valid_start(start_slot, session->duration)) return REASON_NO_FREE_SLOT;
    
    Faculty *fac = (Faculty *)hash_map_get((HashMap*)faculty_map, session->faculty);
    uint64_t eff_mask = 0;
    int max_consec = 0;
    if (fac) {
        eff_mask = fac->unavail_mask;
        max_consec = fac->max_consecutive;
    }
    for (int i=0; i<session->unavail_count; i++) {
        if (session->unavail[i] >= 0 && session->unavail[i] < NUM_SLOTS) {
            eff_mask |= (1ULL << session->unavail[i]);
        }
    }
    
    for (int i=0; i<session->duration; i++) {
        int s = start_slot + i;
        if (eff_mask & (1ULL << s)) return REASON_FACULTY_UNAVAIL;
        if (avail_is_booked(&tracker->faculty, session->faculty, s)) return REASON_NO_FREE_SLOT;
        if (avail_is_booked(&tracker->room, room, s)) return REASON_NO_FREE_SLOT;
        if (session->joint_divisions_count > 0) {
            for (int j=0; j<session->joint_divisions_count; j++) {
                if (avail_is_booked(&tracker->division, session->joint_divisions[j], s)) return REASON_NO_FREE_SLOT;
            }
        } else {
            if (avail_is_booked(&tracker->division, session->division, s)) return REASON_NO_FREE_SLOT;
        }
    }
    
    // Only check faculty consecutive limit, as per user specification.
    if (max_consec > 0) {
        int run_len = session->duration;
        int curr = start_slot;
        while (curr > 0) {
            int prev = curr - 1;
            if (!are_consecutive(prev, curr)) break;
            if (!avail_is_booked(&tracker->faculty, session->faculty, prev)) break;
            run_len++; curr = prev;
        }
        curr = start_slot + session->duration - 1;
        while (curr < NUM_SLOTS - 1) {
            int nxt = curr + 1;
            if (!are_consecutive(curr, nxt)) break;
            if (!avail_is_booked(&tracker->faculty, session->faculty, nxt)) break;
            run_len++; curr = nxt;
        }
        if (run_len > max_consec) return REASON_CONSECUTIVE_LIMIT;
    }
    
    for (int i=0; i<session->duration; i++) {
        int s = start_slot + i;
        avail_book(&tracker->faculty, session->faculty, s, session->id);
        avail_book(&tracker->room, room, s, session->id);
        if (session->joint_divisions_count > 0) {
            for (int j=0; j<session->joint_divisions_count; j++) {
                avail_book(&tracker->division, session->joint_divisions[j], s, session->id);
            }
        } else {
            avail_book(&tracker->division, session->division, s, session->id);
        }
    }
    
    return REASON_NONE;
}

void unplace(AvailTracker *tracker, const Session *session, int start_slot, const char *room) {
    if (start_slot < 0 || start_slot >= NUM_SLOTS) return;
    for (int i=0; i<session->duration; i++) {
        int s = start_slot + i;
        avail_unbook(&tracker->faculty, session->faculty, s);
        avail_unbook(&tracker->room, room, s);
        if (session->joint_divisions_count > 0) {
            for (int j=0; j<session->joint_divisions_count; j++) {
                avail_unbook(&tracker->division, session->joint_divisions[j], s);
            }
        } else {
            avail_unbook(&tracker->division, session->division, s);
        }
    }
}
