#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sqlite3 *db = NULL;


static int column_exists(sqlite3 *mydb, const char *table, const char *column) {
    char sql[256];
    snprintf(sql, sizeof(sql), "PRAGMA table_info(%s);", table);
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(mydb, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    int exists = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(stmt, 1);
        if (strcmp(name, column) == 0) { exists = 1; break; }
    }
    sqlite3_finalize(stmt);
    return exists;
}

int db_init(const char *db_path) {
    if (sqlite3_open(db_path, &db) != SQLITE_OK) return -1;
    
    const char *schema = 
        "CREATE TABLE IF NOT EXISTS rooms ("
        "id TEXT PRIMARY KEY, capacity INTEGER, type INTEGER);"
        
        "CREATE TABLE IF NOT EXISTS faculty ("
        "name TEXT PRIMARY KEY, max_consecutive INTEGER, unavail_mask INTEGER);"
        
        "CREATE TABLE IF NOT EXISTS divisions ("
        "name TEXT PRIMARY KEY, size INTEGER);"
        
        "CREATE TABLE IF NOT EXISTS sessions ("
        "id TEXT PRIMARY KEY, subject TEXT, faculty TEXT, division TEXT, room TEXT, "
        "fixed_slot INTEGER, unavail TEXT);"
        
        "CREATE TABLE IF NOT EXISTS timetable ("
        "session_id TEXT PRIMARY KEY, assigned_slot INTEGER, blocking_reason TEXT);"
        
        "CREATE TABLE IF NOT EXISTS complaints ("
        "id TEXT PRIMARY KEY, target_session_id TEXT, type INTEGER, status INTEGER, "
        "description TEXT, created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "resolved_at DATETIME);";
        
    char *err_msg = NULL;
    if (sqlite3_exec(db, schema, 0, 0, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
        return -1;
    }
    
    /* Phase 1 Migrations */
    const char* session_cols[] = {"student_count INTEGER DEFAULT 0", "duration INTEGER DEFAULT 1", "allowed_rooms TEXT DEFAULT ''", "joint_divisions TEXT DEFAULT ''", "required_type INTEGER DEFAULT 0", "parent_id TEXT DEFAULT ''", "batch_index INTEGER DEFAULT 0", "splittable INTEGER DEFAULT 1"};
    const char* session_col_names[] = {"student_count", "duration", "allowed_rooms", "joint_divisions", "required_type", "parent_id", "batch_index", "splittable"};
    for (int i=0; i<8; i++) {
        if (!column_exists(db, "sessions", session_col_names[i])) {
            char alter[256];
            snprintf(alter, sizeof(alter), "ALTER TABLE sessions ADD COLUMN %s;", session_cols[i]);
            sqlite3_exec(db, alter, 0, 0, 0);
        }
    }
    
    if (!column_exists(db, "timetable", "assigned_room")) {
        sqlite3_exec(db, "ALTER TABLE timetable ADD COLUMN assigned_room TEXT DEFAULT '';", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE timetable ADD COLUMN reason_code INTEGER DEFAULT 0;", 0, 0, 0);
    }
    return 0;
}


void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

int db_save_session(const Session *s) {
    const char *sql = "INSERT OR REPLACE INTO sessions "
                      "(id, subject, faculty, division, room, fixed_slot, unavail, "
                      "student_count, duration, allowed_rooms, joint_divisions, "
                      "required_type, parent_id, batch_index, splittable) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    
    char unavail_str[256] = {0};
    for(int i=0; i<s->unavail_count; i++) {
        char buf[16]; snprintf(buf, sizeof(buf), "%d,", s->unavail[i]); strcat(unavail_str, buf);
    }
    
    char allowed_str[256] = {0};
    for(int i=0; i<s->allowed_rooms_count; i++) {
        strcat(allowed_str, s->allowed_rooms[i]);
        if (i < s->allowed_rooms_count - 1) strcat(allowed_str, ",");
    }
    
    char joint_str[256] = {0};
    for(int i=0; i<s->joint_divisions_count; i++) {
        strcat(joint_str, s->joint_divisions[i]);
        if (i < s->joint_divisions_count - 1) strcat(joint_str, ",");
    }
    
    sqlite3_bind_text(stmt, 1, s->id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, s->subject, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, s->faculty, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, s->division, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, s->room, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, s->fixed_slot);
    sqlite3_bind_text(stmt, 7, unavail_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, s->student_count);
    sqlite3_bind_int(stmt, 9, s->duration);
    sqlite3_bind_text(stmt, 10, allowed_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, joint_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 12, s->required_type);
    sqlite3_bind_text(stmt, 13, s->parent_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 14, s->batch_index);
    sqlite3_bind_int(stmt, 15, s->splittable);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_load_sessions(Session *sessions, int *num_sessions) {
    const char *sql = "SELECT id, subject, faculty, division, room, fixed_slot, unavail, "
                      "student_count, duration, allowed_rooms, joint_divisions, "
                      "required_type, parent_id, batch_index, splittable FROM sessions;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    
    *num_sessions = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *num_sessions < MAX_SESSIONS) {
        Session *s = &sessions[*num_sessions];
        session_init(s);
        
        strncpy(s->id, (const char*)sqlite3_column_text(stmt, 0), MAX_ID_LEN-1);
        strncpy(s->subject, (const char*)sqlite3_column_text(stmt, 1), MAX_NAME_LEN-1);
        strncpy(s->faculty, (const char*)sqlite3_column_text(stmt, 2), MAX_NAME_LEN-1);
        strncpy(s->division, (const char*)sqlite3_column_text(stmt, 3), MAX_NAME_LEN-1);
        const char *rm = (const char*)sqlite3_column_text(stmt, 4);
        if (rm) strncpy(s->room, rm, MAX_NAME_LEN-1);
        s->fixed_slot = sqlite3_column_int(stmt, 5);
        
        const char *u_str = (const char*)sqlite3_column_text(stmt, 6);
        if (u_str) {
            char *copy = strdup(u_str);
            char *tok = strtok(copy, ",");
            while (tok && s->unavail_count < MAX_UNAVAIL) {
                s->unavail[s->unavail_count++] = atoi(tok);
                tok = strtok(NULL, ",");
            }
            free(copy);
        }
        
        s->student_count = sqlite3_column_int(stmt, 7);
        s->duration = sqlite3_column_int(stmt, 8);
        
        const char *a_str = (const char*)sqlite3_column_text(stmt, 9);
        if (a_str) {
            char *copy = strdup(a_str);
            char *tok = strtok(copy, ",");
            while (tok && s->allowed_rooms_count < 5) {
                strncpy(s->allowed_rooms[s->allowed_rooms_count++], tok, MAX_NAME_LEN-1);
                tok = strtok(NULL, ",");
            }
            free(copy);
        }
        
        const char *j_str = (const char*)sqlite3_column_text(stmt, 10);
        if (j_str) {
            char *copy = strdup(j_str);
            char *tok = strtok(copy, ",");
            while (tok && s->joint_divisions_count < 5) {
                strncpy(s->joint_divisions[s->joint_divisions_count++], tok, MAX_NAME_LEN-1);
                tok = strtok(NULL, ",");
            }
            free(copy);
        }
        
        s->required_type = sqlite3_column_int(stmt, 11);
        const char *pid = (const char*)sqlite3_column_text(stmt, 12);
        if (pid) strncpy(s->parent_id, pid, MAX_ID_LEN-1);
        s->batch_index = sqlite3_column_int(stmt, 13);
        s->splittable = sqlite3_column_int(stmt, 14);
        
        (*num_sessions)++;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_save_placement(const SessionPlacement *p) {
    const char *sql = "INSERT OR REPLACE INTO timetable (session_id, assigned_slot, blocking_reason) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    
    sqlite3_bind_text(stmt, 1, p->session_id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, p->assigned_slot);
    sqlite3_bind_text(stmt, 3, p->blocking_reason, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_load_placements(SessionPlacement *placements, int num_sessions) {
    // For each session, find its placement. Defaults to unplaced if not found.
    const char *sql = "SELECT assigned_slot, blocking_reason FROM timetable WHERE session_id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    
    for (int i = 0; i < num_sessions; i++) {
        sqlite3_bind_text(stmt, 1, placements[i].session_id, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            placements[i].assigned_slot = sqlite3_column_int(stmt, 0);
            const char *br = (const char*)sqlite3_column_text(stmt, 1);
            if (br) strncpy(placements[i].blocking_reason, br, MAX_REASON_LEN-1);
        } else {
            placements[i].assigned_slot = -1;
            placements[i].blocking_reason[0] = '\0';
        }
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_save_room(const Room *r) {
    const char *sql = "INSERT OR REPLACE INTO rooms (id, capacity, type) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, r->id, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, r->capacity);
    sqlite3_bind_int(stmt, 3, r->type);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_load_rooms(Room *rooms, int *num_rooms) {
    const char *sql = "SELECT id, capacity, type FROM rooms;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    *num_rooms = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *num_rooms < 100) {
        Room *r = &rooms[*num_rooms];
        strncpy(r->id, (const char*)sqlite3_column_text(stmt, 0), MAX_ID_LEN-1);
        r->capacity = sqlite3_column_int(stmt, 1);
        r->type = sqlite3_column_int(stmt, 2);
        (*num_rooms)++;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_save_faculty(const Faculty *f) {
    const char *sql = "INSERT OR REPLACE INTO faculty (name, max_consecutive, unavail_mask) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, f->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, f->max_consecutive);
    sqlite3_bind_int64(stmt, 3, f->unavail_mask);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_load_faculty(Faculty *faculties, int *num_faculty) {
    const char *sql = "SELECT name, max_consecutive, unavail_mask FROM faculty;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    *num_faculty = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *num_faculty < 100) {
        Faculty *f = &faculties[*num_faculty];
        strncpy(f->name, (const char*)sqlite3_column_text(stmt, 0), MAX_NAME_LEN-1);
        f->max_consecutive = sqlite3_column_int(stmt, 1);
        f->unavail_mask = sqlite3_column_int64(stmt, 2);
        (*num_faculty)++;
    }
    sqlite3_finalize(stmt);
    return 0;
}

int db_save_division(const Division *d) {
    const char *sql = "INSERT OR REPLACE INTO divisions (name, size) VALUES (?, ?);";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, d->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, d->size);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE) ? 0 : -1;
}

int db_load_divisions(Division *divisions, int *num_divs) {
    const char *sql = "SELECT name, size FROM divisions;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    *num_divs = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && *num_divs < 100) {
        Division *d = &divisions[*num_divs];
        strncpy(d->name, (const char*)sqlite3_column_text(stmt, 0), MAX_NAME_LEN-1);
        d->size = sqlite3_column_int(stmt, 1);
        (*num_divs)++;
    }
    sqlite3_finalize(stmt);
    return 0;
}
