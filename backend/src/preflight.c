#include "preflight.h"
#include "slot_helpers.h"
#include <string.h>
#include <stdio.h>

int fit_room(const Session *session, const Room *room) {
    if (session->required_type != room->type) return 0;
    if (room->capacity < session->student_count) return 0;
    
    if (session->allowed_rooms_count > 0) {
        int found = 0;
        for (int i = 0; i < session->allowed_rooms_count; i++) {
            if (strcmp(session->allowed_rooms[i], room->id) == 0) {
                found = 1; break;
            }
        }
        if (!found) return 0;
    }
    return 1;
}

static int get_max_allowed_capacity(const Session *session, const Room *rooms, int num_rooms) {
    int max_cap = -1;
    for (int i = 0; i < num_rooms; i++) {
        if (session->required_type != rooms[i].type) continue;
        if (session->allowed_rooms_count > 0) {
            int found = 0;
            for (int k = 0; k < session->allowed_rooms_count; k++) {
                if (strcmp(session->allowed_rooms[k], rooms[i].id) == 0) { found = 1; break; }
            }
            if (!found) continue;
        }
        if (rooms[i].capacity > max_cap) {
            max_cap = rooms[i].capacity;
        }
    }
    return max_cap;
}

int split_into_batches(Session *sessions, int *num_sessions, const Room *rooms, int num_rooms) {
    if (!sessions || !num_sessions || !rooms) return -1;
    int original_count = *num_sessions;
    for (int i = 0; i < original_count; i++) {
        if (!sessions[i].splittable) continue;
        
        int max_cap = get_max_allowed_capacity(&sessions[i], rooms, num_rooms);
        if (max_cap <= 0) continue; 
        
        if (sessions[i].student_count > max_cap) {
            int num_batches = (sessions[i].student_count + max_cap - 1) / max_cap;
            int base_size = sessions[i].student_count / num_batches;
            int extra = sessions[i].student_count % num_batches;
            
            Session orig = sessions[i];
            
            /* First batch overwrites original */
            sessions[i].student_count = base_size + (0 < extra ? 1 : 0);
            sessions[i].batch_index = 1;
            strcpy(sessions[i].parent_id, orig.id);
            snprintf(sessions[i].id, MAX_ID_LEN, "%s-B1", orig.id);
            
            /* Append the rest */
            for (int b = 1; b < num_batches; b++) {
                if (*num_sessions >= MAX_SESSIONS) return -2; /* Overflow */
                Session *new_s = &sessions[*num_sessions];
                *new_s = orig;
                new_s->student_count = base_size + (b < extra ? 1 : 0);
                new_s->batch_index = b + 1;
                strcpy(new_s->parent_id, orig.id);
                snprintf(new_s->id, MAX_ID_LEN, "%s-B%d", orig.id, b + 1);
                (*num_sessions)++;
            }
        }
    }
    return 0;
}

int preflight_check(const Session *sessions, int num_sessions, const Room *rooms, int num_rooms, const HashMap *faculty_map, SessionPlacement *placements) {
    if (!sessions || !rooms || !faculty_map || !placements) return -1;
    int fail_count = 0;
    
    for (int i = 0; i < num_sessions; i++) {
        placements[i].assigned_slot = -1;
        placements[i].assigned_room[0] = '\0';
        placements[i].reason_code = REASON_NONE;
        strcpy(placements[i].session_id, sessions[i].id);
        
        int fits_any = 0;
        for (int r = 0; r < num_rooms; r++) {
            if (fit_room(&sessions[i], &rooms[r])) {
                fits_any = 1; break;
            }
        }
        
        if (!fits_any) {
            placements[i].reason_code = REASON_CAPACITY;
            snprintf(placements[i].blocking_reason, MAX_REASON_LEN, "no room fits %d students", sessions[i].student_count);
            fail_count++;
            continue;
        }
        
        if (sessions[i].fixed_slot >= 0) {
            if (!valid_start(sessions[i].fixed_slot, sessions[i].duration)) {
                placements[i].reason_code = REASON_FIXED_SLOT_CLASH;
                snprintf(placements[i].blocking_reason, MAX_REASON_LEN, "invalid fixed-slot block");
                fail_count++;
                continue;
            }
            
            Faculty *fac = (Faculty *)hash_map_get(faculty_map, sessions[i].faculty);
            uint64_t eff_mask = 0;
            if (fac) eff_mask = fac->unavail_mask;
            for (int k = 0; k < sessions[i].unavail_count; k++) {
                if (sessions[i].unavail[k] >= 0 && sessions[i].unavail[k] < NUM_SLOTS)
                    eff_mask |= (1ULL << sessions[i].unavail[k]);
            }
            
            int mask_clash = 0;
            for (int d = 0; d < sessions[i].duration; d++) {
                if (eff_mask & (1ULL << (sessions[i].fixed_slot + d))) {
                    mask_clash = 1; break;
                }
            }
            if (mask_clash) {
                placements[i].reason_code = REASON_FACULTY_UNAVAIL;
                snprintf(placements[i].blocking_reason, MAX_REASON_LEN, "fixed_slot inside faculty unavailability");
                fail_count++;
                continue;
            }
        }
    }
    return fail_count; // Returns number of impossible sessions, or <0 on error
}
