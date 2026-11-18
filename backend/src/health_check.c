#include "health_check.h"
#include "avail.h"
#include <string.h>

static int overlaps(int s1_start, int s1_dur, int s2_start, int s2_dur) {
    if (s1_start == -1 || s2_start == -1) return 0;
    int s1_end = s1_start + s1_dur - 1;
    int s2_end = s2_start + s2_dur - 1;
    return (s1_start <= s2_end && s2_start <= s1_end);
}

static int count_consecutive(uint64_t mask, int max_consecutive) {
    if (max_consecutive <= 0) return 0;
    int violations = 0;
    
    for (int d = 0; d < NUM_DAYS; d++) {
        uint64_t day_mask = (mask >> (d * NUM_PERIODS)) & 0x7F;
        uint64_t before = day_mask & 0x0F;
        uint64_t after = (day_mask >> 4) & 0x07;
        
        int run = 0;
        for (int p = 0; p < 4; p++) {
            if ((before >> p) & 1) run++; else run = 0;
            if (run > max_consecutive) violations++;
        }
        
        run = 0;
        for (int p = 0; p < 3; p++) {
            if ((after >> p) & 1) run++; else run = 0;
            if (run > max_consecutive) violations++;
        }
    }
    return violations;
}

void run_health_check(const Session *sessions, int num_sessions, 
                      const Room *rooms, int num_rooms, const HashMap *faculty_map,
                      const SessionPlacement *placements, 
                      HealthCheckReport *report_out) {
    memset(report_out, 0, sizeof(HealthCheckReport));
    
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot == -1) {
            report_out->unplaced_sessions++;
            continue;
        }
        
        const Room *assigned_rm = NULL;
        for (int r = 0; r < num_rooms; r++) {
            if (strcmp(rooms[r].id, placements[i].assigned_room) == 0) {
                assigned_rm = &rooms[r];
                break;
            }
        }
        if (assigned_rm && assigned_rm->capacity < sessions[i].student_count) {
            report_out->capacity_violations++;
        }
        
        const Faculty *fac = (const Faculty *)hash_map_get((HashMap *)faculty_map, sessions[i].faculty);
        uint64_t eff_mask = 0;
        if (fac) eff_mask = fac->unavail_mask;
        for (int u = 0; u < sessions[i].unavail_count; u++) {
            if (sessions[i].unavail[u] >= 0 && sessions[i].unavail[u] < NUM_SLOTS)
                eff_mask |= (1ULL << sessions[i].unavail[u]);
        }
        
        for (int d = 0; d < sessions[i].duration; d++) {
            int st = placements[i].assigned_slot + d;
            if (st >= 0 && st < NUM_SLOTS && ((eff_mask >> st) & 1)) {
                report_out->unavail_violations++;
                break;
            }
        }
        
        for (int j = i + 1; j < num_sessions; j++) {
            if (placements[j].assigned_slot == -1) continue;
            
            if (overlaps(placements[i].assigned_slot, sessions[i].duration, 
                         placements[j].assigned_slot, sessions[j].duration)) {
                
                if (strcmp(sessions[i].faculty, sessions[j].faculty) == 0) {
                    report_out->faculty_conflicts++;
                }
                if (placements[i].assigned_room[0] && 
                    strcmp(placements[i].assigned_room, placements[j].assigned_room) == 0) {
                    report_out->room_conflicts++;
                }
                
                int div_conflict = 0;
                const char *i_divs[10]; int i_count = 0;
                if (sessions[i].joint_divisions_count > 0) {
                    for (int k = 0; k < sessions[i].joint_divisions_count; k++) i_divs[i_count++] = sessions[i].joint_divisions[k];
                } else i_divs[i_count++] = sessions[i].division;
                
                const char *j_divs[10]; int j_count = 0;
                if (sessions[j].joint_divisions_count > 0) {
                    for (int k = 0; k < sessions[j].joint_divisions_count; k++) j_divs[j_count++] = sessions[j].joint_divisions[k];
                } else j_divs[j_count++] = sessions[j].division;
                
                for (int m = 0; m < i_count; m++) {
                    for (int n = 0; n < j_count; n++) {
                        if (strcmp(i_divs[m], j_divs[n]) == 0) {
                            div_conflict = 1;
                        }
                    }
                }
                if (div_conflict) report_out->division_conflicts++;
            }
        }
    }
    
    AvailTracker at;
    if (avail_tracker_init(&at, 1021) == AVAIL_OK) {
        for (int i = 0; i < num_sessions; i++) {
            if (placements[i].assigned_slot != -1) {
                for (int d = 0; d < sessions[i].duration; d++) {
                    int st = placements[i].assigned_slot + d;
                    avail_book(&at.faculty, sessions[i].faculty, st, sessions[i].id);
                }
            }
        }
        
        for (int i = 0; i < at.faculty.num_buckets; i++) {
            for (AvailEntry *e = at.faculty.buckets[i]; e; e = e->next) {
                const Faculty *fac = (const Faculty *)hash_map_get((HashMap *)faculty_map, e->key);
                int max_c = fac ? fac->max_consecutive : 0; 
                // ONLY calculate violations if max_consecutive is strictly > 0
                if (max_c > 0) {
                    report_out->consecutive_violations += count_consecutive(e->booked_mask, max_c);
                }
            }
        }
        avail_tracker_free(&at);
    }
    
    report_out->total_issues = report_out->faculty_conflicts + 
                               report_out->room_conflicts + 
                               report_out->division_conflicts + 
                               report_out->consecutive_violations + 
                               report_out->unplaced_sessions +
                               report_out->unavail_violations +
                               report_out->capacity_violations;
}
