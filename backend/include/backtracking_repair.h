#ifndef BACKTRACKING_REPAIR_H
#define BACKTRACKING_REPAIR_H

#include "graph_coloring.h"

int repair_timetable(Session *sessions, int num_sessions, 
                     const Room *rooms, int num_rooms,
                     const HashMap *faculty_map,
                     SessionPlacement *placements, int max_rounds);

#endif /* BACKTRACKING_REPAIR_H */
