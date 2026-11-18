#include "graph_coloring.h"
#include "preflight.h"
#include "slot_helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Helper struct for qsort */
typedef struct {
    int session_idx;
    int degree;
    int is_fixed;
} SortItem;

static int cmp_sort_item(const void *a, const void *b) {
    const SortItem *ia = (const SortItem *)a;
    const SortItem *ib = (const SortItem *)b;
    
    if (ia->is_fixed != ib->is_fixed) {
        return ib->is_fixed - ia->is_fixed;
    }
    if (ia->degree != ib->degree) {
        return ib->degree - ia->degree;
    }
    return ia->session_idx - ib->session_idx;
}

int color_timetable(Session *sessions, int num_sessions, 
                    const Room *rooms, int num_rooms,
                    const HashMap *faculty_map,
                    SlotOrdering mode, SessionPlacement *result_array) {
    if (num_sessions <= 0) return 0;
    
    /* Preflight check handles obviously impossible ones */
    preflight_check(sessions, num_sessions, rooms, num_rooms, faculty_map, result_array);
    
    ConflictGraph g;
    if (graph_build(&g, sessions, num_sessions) != 0) {
        return -1;
    }
    
    AvailTracker at;
    if (avail_tracker_init(&at, 1021) != AVAIL_OK) {
        graph_free(&g);
        return -1;
    }
    
    SortItem *items = malloc(num_sessions * sizeof(SortItem));
    if (!items) {
        avail_tracker_free(&at);
        graph_free(&g);
        return -1;
    }
    
    for (int i = 0; i < num_sessions; i++) {
        items[i].session_idx = i;
        items[i].degree = graph_degree(&g, i);
        items[i].is_fixed = (sessions[i].fixed_slot != -1) ? 1 : 0;
    }
    
    qsort(items, num_sessions, sizeof(SortItem), cmp_sort_item);
    
    int slot_order[NUM_SLOTS];
    if (mode == ORDERING_EARLIEST) {
        for (int i = 0; i < NUM_SLOTS; i++) slot_order[i] = i;
    } else {
        int idx = 0;
        for (int p = 0; p < NUM_PERIODS; p++) {
            for (int d = 0; d < NUM_DAYS; d++) {
                slot_order[idx++] = SLOT(d, p);
            }
        }
    }
    
    for (int i = 0; i < num_sessions; i++) {
        int s_idx = items[i].session_idx;
        Session *s = &sessions[s_idx];
        
        if (result_array[s_idx].reason_code != REASON_NONE && result_array[s_idx].reason_code != REASON_NO_FREE_SLOT) {
            /* Handled by preflight */
            continue;
        }
        
        if (items[i].is_fixed) {
            int slot = s->fixed_slot;
            
            int placed = 0;
            UnplacedReason last_reason = REASON_NO_ROOM;
            for (int r = 0; r < num_rooms; r++) {
                if (!fit_room(s, &rooms[r])) continue;
                
                UnplacedReason reason = try_place(&at, faculty_map, s, slot, rooms[r].id);
                if (reason == REASON_NONE) {
                    result_array[s_idx].assigned_slot = slot;
                    strcpy(result_array[s_idx].assigned_room, rooms[r].id);
                    result_array[s_idx].reason_code = REASON_NONE;
                    result_array[s_idx].blocking_reason[0] = '\0';
                    placed = 1;
                    break;
                }
                last_reason = reason;
            }
            if (!placed) {
                result_array[s_idx].reason_code = last_reason;
                snprintf(result_array[s_idx].blocking_reason, MAX_REASON_LEN, "Fixed slot conflict");
            }
            continue;
        }
        
        /* Greedy placement */
        int placed = 0;
        UnplacedReason last_reason = REASON_NO_FREE_SLOT;
        for (int j = 0; j < NUM_SLOTS; j++) {
            int slot = slot_order[j];
            
            for (int r = 0; r < num_rooms; r++) {
                if (!fit_room(s, &rooms[r])) continue;
                
                UnplacedReason reason = try_place(&at, faculty_map, s, slot, rooms[r].id);
                if (reason == REASON_NONE) {
                    result_array[s_idx].assigned_slot = slot;
                    strcpy(result_array[s_idx].assigned_room, rooms[r].id);
                    result_array[s_idx].reason_code = REASON_NONE;
                    result_array[s_idx].blocking_reason[0] = '\0';
                    placed = 1;
                    break;
                } else if (reason != REASON_NO_FREE_SLOT) {
                    last_reason = reason; 
                }
            }
            if (placed) break;
        }
        
        if (!placed) {
            result_array[s_idx].reason_code = last_reason;
            snprintf(result_array[s_idx].blocking_reason, MAX_REASON_LEN, "No valid slot/room available");
        }
    }
    
    free(items);
    avail_tracker_free(&at);
    graph_free(&g);
    return 0;
}
