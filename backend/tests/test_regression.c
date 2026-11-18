#include "json_loader.h"
#include "db.h"
#include "hash_map.h"
#include "graph_coloring.h"
#include "backtracking_repair.h"
#include "health_check.h"
#include "preflight.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("=== PHASE 8: Regression Test ===\n");
    unlink("test_regression.db");
    assert(db_init("test_regression.db") == 0);
    assert(load_adversarial_dataset("tests/data/adversarial.json") == 0);
    
    Room rooms[100]; int num_rooms = 0;
    db_load_rooms(rooms, &num_rooms);
    
    Faculty faculties[100]; int num_faculty = 0;
    db_load_faculty(faculties, &num_faculty);
    
    HashMap map; hash_map_init(&map, 101);
    for(int i=0; i<num_faculty; i++) {
        hash_map_put(&map, faculties[i].name, &faculties[i]);
    }
    
    Session sessions[400]; int num_sessions = 0;
    db_load_sessions(sessions, &num_sessions);
    
    // Batch Split preflight
    split_into_batches(sessions, &num_sessions, rooms, num_rooms);
    
    SessionPlacement placements[400];
    for (int i = 0; i < num_sessions; i++) {
        memset(&placements[i], 0, sizeof(SessionPlacement));
        strcpy(placements[i].session_id, sessions[i].id);
        placements[i].assigned_slot = -1;
    }
    
    printf("Generating timetable for %d distinct session components...\n", num_sessions);
    
    color_timetable(sessions, num_sessions, rooms, num_rooms, &map, ORDERING_SPREAD, placements);
    repair_timetable(sessions, num_sessions, rooms, num_rooms, &map, placements, 20000);
    
    HealthCheckReport r;
    run_health_check(sessions, num_sessions, rooms, num_rooms, &map, placements, &r);
    
    printf("\nHealth Check Report:\n");
    printf("  Unplaced Sessions: %d\n", r.unplaced_sessions);
    printf("  Faculty Conflicts: %d\n", r.faculty_conflicts);
    printf("  Room Conflicts:    %d\n", r.room_conflicts);
    printf("  Division Conflicts: %d\n", r.division_conflicts);
    printf("  Consecutive Viol.: %d\n", r.consecutive_violations);
    printf("  Unavail Viol.:     %d\n", r.unavail_violations);
    printf("  Capacity Viol.:    %d\n", r.capacity_violations);
    printf("  Total Issues:      %d\n", r.total_issues);
    
    for (int i=0; i<num_sessions; i++) {
        if (placements[i].assigned_slot == -1) {
            printf("UNPLACED: %s (Reason %d: %s)\n", placements[i].session_id, 
                   placements[i].reason_code, placements[i].blocking_reason);
        }
    }
    
    assert(r.total_issues == 1);
    assert(r.unplaced_sessions == 1);
    
    hash_map_free(&map);
    db_close();
    FILE *f = fopen("tables.json", "w");
    fprintf(f, "[\n");
    for (int i = 0; i < num_sessions; i++) {
        fprintf(f, "{\"id\":\"%s\", \"subj\":\"%s\", \"fac\":\"%s\", \"div\":\"%s\", \"dur\":%d, \"slot\":%d, \"room\":\"%s\", \"jc\":%d}%s\n",
                sessions[i].id, sessions[i].subject, sessions[i].faculty, sessions[i].division, 
                sessions[i].duration, placements[i].assigned_slot, placements[i].assigned_room,
                sessions[i].joint_divisions_count, i == num_sessions-1 ? "" : ",");
    }
    fprintf(f, "]\n");
    fclose(f);

    printf("\n[PASS] Adversarial regression complete.\n");
    return 0;
}
