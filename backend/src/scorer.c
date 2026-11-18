#include "scorer.h"
#include "avail.h"
#include <string.h>

static int highest_bit_set(uint64_t mask) {
    for (int i = 6; i >= 0; i--) {
        if ((mask >> i) & 1) return i;
    }
    return -1;
}

static int lowest_bit_set(uint64_t mask) {
    for (int i = 0; i <= 6; i++) {
        if ((mask >> i) & 1) return i;
    }
    return -1;
}

static int count_gaps_in_mask(uint64_t mask) {
    int total_gaps = 0;
    for (int d = 0; d < NUM_DAYS; d++) {
        uint64_t day_mask = (mask >> (d * NUM_PERIODS)) & 0x7F;
        if (day_mask == 0) continue;
        
        int high = highest_bit_set(day_mask);
        int low = lowest_bit_set(day_mask);
        
        int span = (high - low) + 1;
        
        int sessions = 0;
        for (int i = low; i <= high; i++) {
            if ((day_mask >> i) & 1) sessions++;
        }
        
        total_gaps += (span - sessions);
    }
    return total_gaps;
}

int evaluate_timetable(const Session *sessions, int num_sessions,
                       const SessionPlacement *placements, 
                       TimetableScore *score_out) {
    
    memset(score_out, 0, sizeof(TimetableScore));
    
    score_out->is_complete = 1;
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot == -1) {
            score_out->is_complete = 0;
            break; // Used to return -1 here. Now we just set the flag and continue.
        }
    }
    
    if (num_sessions == 0) {
        score_out->gap_score = 100.0f;
        score_out->balance_score = 100.0f;
        score_out->util_score = 0.0f;
        score_out->total_score = 80.0f; 
        return 0;
    }
    
    AvailTracker at;
    if (avail_tracker_init(&at, 1021) != AVAIL_OK) return -1;
    
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot == -1) continue; // Skip unplaced
        
        for (int d = 0; d < sessions[i].duration; d++) {
            int st = placements[i].assigned_slot + d;
            avail_book(&at.faculty, sessions[i].faculty, st, sessions[i].id);
            if (placements[i].assigned_room[0]) {
                avail_book(&at.room, placements[i].assigned_room, st, sessions[i].id);
            }
            if (sessions[i].joint_divisions_count > 0) {
                for (int j = 0; j < sessions[i].joint_divisions_count; j++) {
                    avail_book(&at.division, sessions[i].joint_divisions[j], st, sessions[i].id);
                }
            } else {
                avail_book(&at.division, sessions[i].division, st, sessions[i].id);
            }
        }
    }
    
    int total_gaps = 0;
    for (int i = 0; i < at.faculty.num_buckets; i++) {
        for (AvailEntry *e = at.faculty.buckets[i]; e; e = e->next) {
            total_gaps += count_gaps_in_mask(e->booked_mask);
        }
    }
    for (int i = 0; i < at.division.num_buckets; i++) {
        for (AvailEntry *e = at.division.buckets[i]; e; e = e->next) {
            total_gaps += count_gaps_in_mask(e->booked_mask);
        }
    }
    avail_tracker_free(&at);
    
    score_out->gap_score = 100.0f - (total_gaps * 2.0f);
    if (score_out->gap_score < 0.0f) score_out->gap_score = 0.0f;
    
    int day_counts[NUM_DAYS] = {0};
    int placed_count = 0;
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot != -1) {
            day_counts[SLOT_DAY(placements[i].assigned_slot)]++;
            placed_count++;
        }
    }
    
    if (placed_count == 0) {
        score_out->balance_score = 100.0f;
        score_out->util_score = 0.0f;
        score_out->gap_score = 100.0f;
        score_out->total_score = 80.0f;
        return 0;
    }
    
    int max_c = 0, min_c = placed_count;
    for (int d = 0; d < NUM_DAYS; d++) {
        if (day_counts[d] > max_c) max_c = day_counts[d];
        if (day_counts[d] < min_c) min_c = day_counts[d];
    }
    int spread = max_c - min_c;
    
    score_out->balance_score = 100.0f - (spread * 5.0f);
    if (score_out->balance_score < 0.0f) score_out->balance_score = 0.0f;
    
    int active_slots[NUM_SLOTS] = {0};
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot != -1) {
            active_slots[placements[i].assigned_slot] = 1;
        }
    }
    int used_slots = 0;
    for (int i = 0; i < NUM_SLOTS; i++) used_slots += active_slots[i];
    
    score_out->util_score = ((float)used_slots / (float)NUM_SLOTS) * 100.0f;
    
    score_out->total_score = (score_out->gap_score * 0.4f) + 
                             (score_out->balance_score * 0.4f) + 
                             (score_out->util_score * 0.2f);
                             
    return 0;
}
