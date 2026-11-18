#include "complaints.h"
#include "backtracking_repair.h"
#include <string.h>
#include <stdlib.h>

void complaint_init(Complaint *c, const char *id, const char *target_session, 
                    ComplaintType type, const char *desc) {
    strncpy(c->id, id, MAX_ID_LEN - 1);
    c->id[MAX_ID_LEN - 1] = '\0';
    
    strncpy(c->target_session_id, target_session, MAX_ID_LEN - 1);
    c->target_session_id[MAX_ID_LEN - 1] = '\0';
    
    c->type = type;
    c->status = STATUS_PENDING;
    
    strncpy(c->description, desc, MAX_REASON_LEN - 1);
    c->description[MAX_REASON_LEN - 1] = '\0';
    
    c->proposed_timetable = NULL;
    c->num_placements = 0;
}

static int apply_fix_logic(const char *target_id, Session *sessions, int num_sessions, 
                           const Room *rooms, int num_rooms, const HashMap *faculty_map,
                           SessionPlacement *placements) {
    int target_idx = -1;
    for (int i = 0; i < num_sessions; i++) {
        if (strcmp(sessions[i].id, target_id) == 0) {
            target_idx = i;
            break;
        }
    }
    
    if (target_idx == -1) return -1;
    
    int current_slot = placements[target_idx].assigned_slot;
    if (current_slot == -1) return -1;
    
    int u_idx = sessions[target_idx].unavail_count;
    int slot_forbidden = 0;
    
    if (u_idx < MAX_UNAVAIL) {
        sessions[target_idx].unavail[u_idx] = current_slot;
        sessions[target_idx].unavail_count++;
        slot_forbidden = 1;
    }
    
    placements[target_idx].assigned_slot = -1;
    placements[target_idx].assigned_room[0] = '\0';
    strncpy(placements[target_idx].blocking_reason, "Complaint repair unplace", MAX_REASON_LEN - 1);
    placements[target_idx].blocking_reason[MAX_REASON_LEN - 1] = '\0';
    
    int repair_res = repair_timetable(sessions, num_sessions, rooms, num_rooms, faculty_map, placements, 100);
    
    if (slot_forbidden) {
        sessions[target_idx].unavail_count--;
    }
    
    if (repair_res != 0) return -1;
    
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot == -1) return -1;
    }
    
    if (placements[target_idx].assigned_slot == current_slot) return -1;
    
    return 0;
}

int complaint_generate_proposal(Complaint *c, Session *sessions, int num_sessions, 
                                const Room *rooms, int num_rooms, const HashMap *faculty_map,
                                const SessionPlacement *live_placements) {
    if (c->status != STATUS_PENDING) return -1;
    
    if (c->proposed_timetable) {
        free(c->proposed_timetable);
    }
    
    c->num_placements = num_sessions;
    c->proposed_timetable = malloc(num_sessions * sizeof(SessionPlacement));
    if (!c->proposed_timetable) return -1;
    
    memcpy(c->proposed_timetable, live_placements, num_sessions * sizeof(SessionPlacement));
    
    if (apply_fix_logic(c->target_session_id, sessions, num_sessions, rooms, num_rooms, faculty_map, c->proposed_timetable) == 0) {
        c->status = STATUS_PROPOSED;
        return 0;
    } else {
        c->status = STATUS_FAILED;
        free(c->proposed_timetable);
        c->proposed_timetable = NULL;
        return -1;
    }
}

void complaint_approve(Complaint *c) {
    if (c->status == STATUS_PROPOSED) {
        c->status = STATUS_APPROVED;
    }
}

void complaint_reject(Complaint *c) {
    if (c->status == STATUS_PROPOSED) {
        c->status = STATUS_REJECTED;
    }
}

int complaint_commit(Complaint *c, Session *sessions, int num_sessions, 
                     const Room *rooms, int num_rooms, const HashMap *faculty_map,
                     SessionPlacement *live_placements) {
    if (c->status != STATUS_APPROVED) return -1;
    
    SessionPlacement *copy = malloc(num_sessions * sizeof(SessionPlacement));
    if (!copy) return -1;
    
    memcpy(copy, live_placements, num_sessions * sizeof(SessionPlacement));
    
    if (apply_fix_logic(c->target_session_id, sessions, num_sessions, rooms, num_rooms, faculty_map, copy) == 0) {
        memcpy(live_placements, copy, num_sessions * sizeof(SessionPlacement));
        c->status = STATUS_RESOLVED;
        free(copy);
        return 0;
    } else {
        c->status = STATUS_FAILED;
        free(copy);
        return -1;
    }
}

void complaint_free(Complaint *c) {
    if (c->proposed_timetable) {
        free(c->proposed_timetable);
        c->proposed_timetable = NULL;
    }
}
