#include "excel_export.h"
#include <stdio.h>
#include <string.h>

#if defined(__has_include) && __has_include(<xlsxwriter.h>)
#include <xlsxwriter.h>

int export_timetable_to_excel(const char *filename, 
                              const Session *sessions, int num_sessions, 
                              const SessionPlacement *placements) {
    lxw_workbook  *workbook  = workbook_new(filename);
    if (!workbook) return -1;
    
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, "Timetable");
    
    lxw_format *header_format = workbook_add_format(workbook);
    format_set_bold(header_format);
    format_set_bg_color(header_format, 0xCCCCCC);
    
    worksheet_write_string(worksheet, 0, 0, "Day \\ Period", header_format);
    for (int p = 0; p < NUM_PERIODS; p++) {
        char header[32];
        snprintf(header, sizeof(header), "P%d (%s-%s)", p+1, PERIOD_START[p], PERIOD_END[p]);
        worksheet_write_string(worksheet, 0, p + 1, header, header_format);
    }
    
    for (int d = 0; d < NUM_DAYS; d++) {
        worksheet_write_string(worksheet, d + 1, 0, DAY_NAMES[d], header_format);
    }
    
    lxw_worksheet *dump = workbook_add_worksheet(workbook, "Master List");
    worksheet_write_string(dump, 0, 0, "Session ID", header_format);
    worksheet_write_string(dump, 0, 1, "Subject", header_format);
    worksheet_write_string(dump, 0, 2, "Faculty", header_format);
    worksheet_write_string(dump, 0, 3, "Division", header_format);
    worksheet_write_string(dump, 0, 4, "Room", header_format);
    worksheet_write_string(dump, 0, 5, "Day", header_format);
    worksheet_write_string(dump, 0, 6, "Period", header_format);
    
    int row = 1;
    for (int i = 0; i < num_sessions; i++) {
        if (placements[i].assigned_slot != -1) {
            int slot = placements[i].assigned_slot;
            int day = SLOT_DAY(slot);
            int period = SLOT_PERIOD(slot);
            
            worksheet_write_string(dump, row, 0, sessions[i].id, NULL);
            worksheet_write_string(dump, row, 1, sessions[i].subject, NULL);
            worksheet_write_string(dump, row, 2, sessions[i].faculty, NULL);
            worksheet_write_string(dump, row, 3, sessions[i].division, NULL);
            worksheet_write_string(dump, row, 4, sessions[i].room, NULL);
            worksheet_write_string(dump, row, 5, DAY_NAMES[day], NULL);
            worksheet_write_number(dump, row, 6, period + 1, NULL);
            row++;
        }
    }
    
    return workbook_close(workbook) == LXW_NO_ERROR ? 0 : -1;
}

#else
// Fallback if libxlsxwriter is not installed yet
int export_timetable_to_excel(const char *filename, 
                              const Session *sessions, int num_sessions, 
                              const SessionPlacement *placements) {
    (void)sessions; (void)num_sessions; (void)placements;
    printf("WARN: Cannot export to %s. Please install libxlsxwriter (brew install libxlsxwriter) and recompile.\n", filename);
    return -1;
}
#endif
