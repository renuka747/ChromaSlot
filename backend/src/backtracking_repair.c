#include "backtracking_repair.h"
#include "slot_helpers.h"
#include "preflight.h"
#include <string.h>

static int evaluate_placements(SessionPlacement *placements, int num_sessions) {
    int unplaced = 0;
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot == -1) unplaced++;
    }
    return unplaced;
}

static int find_session(Session *sessions, int num_sessions, const char *id) {
    for (int i = 0; i < num_sessions; i++) {
        if (strcmp(sessions[i].id, id) == 0) return i;
    }
    return -1;
}

static int get_single_evictable_blocker(AvailTracker *at, Session *s, int slot, const char *room,
                                        Session *sessions, int num_sessions) {
    int blockers[10];
    int count = 0;
    
    for (int d = 0; d < s->duration; d++) {
        int st = slot + d;
        const char *fac = avail_booker(&at->faculty, s->faculty, st);
        if (fac) {
            int idx = find_session(sessions, num_sessions, fac);
            if (idx != -1) blockers[count++] = idx;
        }
        const char *rm = avail_booker(&at->room, room, st);
        if (rm) {
            int idx = find_session(sessions, num_sessions, rm);
            if (idx != -1) blockers[count++] = idx;
        }
        if (s->joint_divisions_count > 0) {
            for (int j = 0; j < s->joint_divisions_count; j++) {
                const char *div = avail_booker(&at->division, s->joint_divisions[j], st);
                if (div) {
                    int idx = find_session(sessions, num_sessions, div);
                    if (idx != -1) blockers[count++] = idx;
                }
            }
        } else {
            const char *div = avail_booker(&at->division, s->division, st);
            if (div) {
                int idx = find_session(sessions, num_sessions, div);
                if (idx != -1) blockers[count++] = idx;
            }
        }
    }
    
    if (count == 0) return -1;
    
    int b_idx = blockers[0];
    for (int i = 1; i < count; i++) {
        if (blockers[i] != b_idx) return -1;
    }
    
    if (sessions[b_idx].fixed_slot != -1) return -1;
    return b_idx;
}

static void greedy_place(AvailTracker *at, Session *s, SessionPlacement *p, 
                         const Room *rooms, int num_rooms, const HashMap *faculty_map) {
    int start = (s->fixed_slot != -1) ? s->fixed_slot : 0;
    int end = (s->fixed_slot != -1) ? s->fixed_slot : (NUM_SLOTS - 1);
    
    for (int slot = start; slot <= end; slot++) {
        for (int r = 0; r < num_rooms; r++) {
            if (!fit_room(s, &rooms[r])) continue;
            
            if (try_place(at, faculty_map, s, slot, rooms[r].id) == REASON_NONE) {
                p->assigned_slot = slot;
                strcpy(p->assigned_room, rooms[r].id);
                p->reason_code = REASON_NONE;
                p->blocking_reason[0] = '\0';
                return;
            }
        }
    }
    p->assigned_slot = -1;
    p->assigned_room[0] = '\0';
    p->reason_code = REASON_NO_FREE_SLOT;
    strncpy(p->blocking_reason, "Evicted, no free slot", MAX_REASON_LEN - 1);
}

int repair_timetable(Session *sessions, int num_sessions, 
                     const Room *rooms, int num_rooms, const HashMap *faculty_map,
                     SessionPlacement *placements, int max_rounds) {
    AvailTracker at;
    if (avail_tracker_init(&at, 1021) != AVAIL_OK) return -1;
    
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot != -1) {
            try_place(&at, faculty_map, &sessions[i], placements[i].assigned_slot, placements[i].assigned_room);
        }
    }
    
    int best_cost = evaluate_placements(placements, num_sessions);
    int failed_attempts = 0;
    int curr_session = 0;
    
    while (failed_attempts < max_rounds && best_cost > 0) {
        int i = curr_session;
        curr_session = (curr_session + 1) % num_sessions;
        int mutated = 0;
        
        if (placements[i].assigned_slot == -1) {
            int start = (sessions[i].fixed_slot != -1) ? sessions[i].fixed_slot : 0;
            int end = (sessions[i].fixed_slot != -1) ? sessions[i].fixed_slot : (NUM_SLOTS - 1);
            int placed = 0;
            
            for (int slot = start; slot <= end; slot++) {
                for (int r = 0; r < num_rooms; r++) {
                    if (!fit_room(&sessions[i], &rooms[r])) continue;
                    
                    if (try_place(&at, faculty_map, &sessions[i], slot, rooms[r].id) == REASON_NONE) {
                        placements[i].assigned_slot = slot;
                        strcpy(placements[i].assigned_room, rooms[r].id);
                        placements[i].reason_code = REASON_NONE;
                        
                        int new_cost = evaluate_placements(placements, num_sessions);
                        if (new_cost < best_cost) {
                            best_cost = new_cost;
                            mutated = 1;
                            placed = 1;
                            break;
                        }
                        unplace(&at, &sessions[i], slot, rooms[r].id);
                        placements[i].assigned_slot = -1;
                        placements[i].assigned_room[0] = '\0';
                    }
                }
                if (placed) break;
            }
            
            if (placed) {
                failed_attempts = 0;
                continue;
            }
            
            for (int slot = start; slot <= end; slot++) {
                for (int r = 0; r < num_rooms; r++) {
                    if (!fit_room(&sessions[i], &rooms[r])) continue;
                    
                    int blocker = get_single_evictable_blocker(&at, &sessions[i], slot, rooms[r].id, sessions, num_sessions);
                    if (blocker != -1) {
                        SessionPlacement p_backup = placements[blocker];
                        SessionPlacement i_backup = placements[i];
                        
                        unplace(&at, &sessions[blocker], placements[blocker].assigned_slot, placements[blocker].assigned_room);
                        placements[blocker].assigned_slot = -1;
                        placements[blocker].assigned_room[0] = '\0';
                        
                        if (try_place(&at, faculty_map, &sessions[i], slot, rooms[r].id) == REASON_NONE) {
                            placements[i].assigned_slot = slot;
                            strcpy(placements[i].assigned_room, rooms[r].id);
                            
                            greedy_place(&at, &sessions[blocker], &placements[blocker], rooms, num_rooms, faculty_map);
                            
                            int new_cost = evaluate_placements(placements, num_sessions);
                            if (new_cost < best_cost) {
                                best_cost = new_cost;
                                mutated = 1;
                                break;
                            }
                            
                            if (placements[blocker].assigned_slot != -1) {
                                unplace(&at, &sessions[blocker], placements[blocker].assigned_slot, placements[blocker].assigned_room);
                            }
                            unplace(&at, &sessions[i], slot, rooms[r].id);
                        }
                        
                        placements[blocker] = p_backup;
                        placements[i] = i_backup;
                        
                        if (p_backup.assigned_slot != -1) {
                            try_place(&at, faculty_map, &sessions[blocker], p_backup.assigned_slot, p_backup.assigned_room);
                        }
                    }
                }
                if (mutated) break;
            }
        }
        
        if (mutated) {
            failed_attempts = 0;
        } else {
            failed_attempts++;
        }
    }
    
    avail_tracker_free(&at);
    return 0;
}
