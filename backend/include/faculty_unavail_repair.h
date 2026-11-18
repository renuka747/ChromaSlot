#ifndef FACULTY_UNAVAIL_REPAIR_H
#define FACULTY_UNAVAIL_REPAIR_H

#include "graph_coloring.h"
#include "hash_map.h"

int mark_faculty_unavailable(const char *faculty_name, 
                             const int *new_unavail, int count,
                             Session *sessions, int num_sessions,
                             const Room *rooms, int num_rooms,
                             HashMap *faculty_map, /* Non-const because M6 edits Faculty.unavail_mask */
                             SessionPlacement *placements);

#endif /* FACULTY_UNAVAIL_REPAIR_H */
