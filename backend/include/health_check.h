#ifndef HEALTH_CHECK_H
#define HEALTH_CHECK_H

#include "graph_coloring.h"
#include "hash_map.h"

typedef struct {
    int faculty_conflicts;
    int room_conflicts;
    int division_conflicts;
    int consecutive_violations;
    int unplaced_sessions;
    int unavail_violations;
    int capacity_violations;
    int total_issues;
} HealthCheckReport;

void run_health_check(const Session *sessions, int num_sessions, 
                      const Room *rooms, int num_rooms, const HashMap *faculty_map,
                      const SessionPlacement *placements, 
                      HealthCheckReport *report_out);

#endif /* HEALTH_CHECK_H */
