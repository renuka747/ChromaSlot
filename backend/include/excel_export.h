#ifndef EXCEL_EXPORT_H
#define EXCEL_EXPORT_H

#include "graph_coloring.h"

/*
 * export_timetable_to_excel()
 * 
 * Generates an Excel file mapping the timetable into a human-readable grid.
 * Requires libxlsxwriter.
 * 
 * Returns 0 on success, -1 on failure.
 */
int export_timetable_to_excel(const char *filename, 
                              const Session *sessions, int num_sessions, 
                              const SessionPlacement *placements);

#endif
