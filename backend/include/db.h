#ifndef DB_H
#define DB_H

#include "graph_coloring.h"
#include "complaints.h"
#include <sqlite3.h>

int db_init(const char *db_path);
void db_close(void);

int db_save_session(const Session *s);
int db_load_sessions(Session *sessions, int *num_sessions);

int db_save_placement(const SessionPlacement *p);
int db_load_placements(SessionPlacement *placements, int num_sessions);

int db_save_complaint(const Complaint *c);
int db_load_complaints(Complaint *complaints, int *num_complaints);

#endif
int db_save_room(const Room *r);
int db_load_rooms(Room *rooms, int *num_rooms);
int db_save_faculty(const Faculty *f);
int db_load_faculty(Faculty *faculties, int *num_faculty);
int db_save_division(const Division *d);
int db_load_divisions(Division *divisions, int *num_divs);
