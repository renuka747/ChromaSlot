#ifndef GRAPH_COLORING_H
#define GRAPH_COLORING_H

#include "constants.h"
#include "graph_builder.h"
#include "core_types.h"
#include "hash_map.h"

typedef enum {
    ORDERING_EARLIEST,
    ORDERING_SPREAD
} SlotOrdering;

/*
 * color_timetable() — Assign a slot to every session greedily using try_place.
 * 
 * Returns 0 on success, -1 on memory allocation error.
 */
int color_timetable(Session *sessions, int num_sessions, 
                    const Room *rooms, int num_rooms,
                    const HashMap *faculty_map,
                    SlotOrdering mode, SessionPlacement *result_array);

#endif /* GRAPH_COLORING_H */
