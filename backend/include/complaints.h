#ifndef COMPLAINTS_H
#define COMPLAINTS_H

#include "graph_coloring.h"
#include "hash_map.h"

typedef enum {
    COMPLAINT_GAP,
    COMPLAINT_SHIFT,
    COMPLAINT_OTHER
} ComplaintType;

typedef enum {
    STATUS_PENDING,
    STATUS_PROPOSED,
    STATUS_APPROVED,
    STATUS_REJECTED,
    STATUS_RESOLVED,
    STATUS_FAILED
} ComplaintStatus;

typedef struct {
    char id[MAX_ID_LEN];
    char target_session_id[MAX_ID_LEN];
    ComplaintType type;
    ComplaintStatus status;
    char description[MAX_REASON_LEN];
    
    SessionPlacement *proposed_timetable;
    int num_placements;
} Complaint;

void complaint_init(Complaint *c, const char *id, const char *target_session, 
                    ComplaintType type, const char *desc);

int complaint_generate_proposal(Complaint *c, Session *sessions, int num_sessions, 
                                const Room *rooms, int num_rooms, const HashMap *faculty_map,
                                const SessionPlacement *live_placements);

void complaint_approve(Complaint *c);
void complaint_reject(Complaint *c);

int complaint_commit(Complaint *c, Session *sessions, int num_sessions, 
                     const Room *rooms, int num_rooms, const HashMap *faculty_map,
                     SessionPlacement *live_placements);

void complaint_free(Complaint *c);

#endif /* COMPLAINTS_H */
