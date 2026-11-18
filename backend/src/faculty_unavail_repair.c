#include "faculty_unavail_repair.h"
#include "backtracking_repair.h"
#include <string.h>

int mark_faculty_unavailable(const char *faculty_name, 
                             const int *new_unavail, int count,
                             Session *sessions, int num_sessions,
                             const Room *rooms, int num_rooms,
                             HashMap *faculty_map,
                             SessionPlacement *placements) {
    if (count <= 0) return 0;
    
    Faculty *fac = (Faculty *)hash_map_get(faculty_map, faculty_name);
    if (fac) {
        for (int k = 0; k < count; k++) {
            int slot = new_unavail[k];
            if (slot >= 0 && slot < NUM_SLOTS) {
                fac->unavail_mask |= (1ULL << slot);
            }
        }
    }
    
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot != -1 && 
            strcmp(sessions[i].faculty, faculty_name) == 0) {
            
            int current_slot = placements[i].assigned_slot;
            int conflict = 0;
            for (int k = 0; k < count; k++) {
                if (current_slot == new_unavail[k]) {
                    conflict = 1;
                    break;
                }
            }
            
            if (conflict) {
                placements[i].assigned_slot = -1;
                placements[i].assigned_room[0] = '\0';
                placements[i].reason_code = REASON_FACULTY_UNAVAIL;
                strncpy(placements[i].blocking_reason, "Faculty newly unavailable", MAX_REASON_LEN - 1);
                placements[i].blocking_reason[MAX_REASON_LEN - 1] = '\0';
            }
        }
    }
    
    int repair_res = repair_timetable(sessions, num_sessions, rooms, num_rooms, faculty_map, placements, 100);
    if (repair_res != 0) return -1;
    
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot == -1) return -1;
    }
    
    return 0;
}
