#include "core_types.h"
#include "cJSON/cJSON.h"
#include "mongoose/mongoose.h"
#include "server.h"
#include "graph_coloring.h"
#include "backtracking_repair.h"
#include "scorer.h"
#include "faculty_unavail_repair.h"
#include "preflight.h"
#include "health_check.h"
#include "json_loader.h"
#include "db.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Room global_rooms[100];
int global_num_rooms;
Faculty global_faculty[100];
int global_num_faculty;
Division global_divisions[100];
int global_num_divisions;
Session global_sessions[1000];
int global_num_sessions;
HashMap global_faculty_map;
SessionPlacement global_placements[1000];

static void send_json(struct mg_connection *c, cJSON *json) {
    char *str = cJSON_PrintUnformatted(json);
    mg_http_reply(c, 200, "Content-Type: application/json\r\nCache-Control: no-cache, no-store, must-revalidate\r\n", "%s", str);
    free(str);
    cJSON_Delete(json);
}

static cJSON* build_timetable_json() {
    cJSON *resp = cJSON_CreateObject();
    cJSON *placements_arr = cJSON_CreateArray();
    cJSON *unplaced_arr = cJSON_CreateArray();
    
    for(int i=0; i<global_num_sessions; i++) {
        Session *s = &global_sessions[i];
        if (global_placements[i].assigned_slot == -1) {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "session_id", global_placements[i].session_id);
            cJSON_AddNumberToObject(item, "code", global_placements[i].reason_code);
            cJSON_AddStringToObject(item, "message", global_placements[i].blocking_reason);
            cJSON_AddStringToObject(item, "subject", s->subject);
            cJSON_AddStringToObject(item, "faculty", s->faculty);
            cJSON_AddStringToObject(item, "division", s->joint_divisions_count > 0 ? "Joint" : s->division);
            cJSON_AddItemToArray(unplaced_arr, item);
        } else {
            cJSON *item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "session_id", global_placements[i].session_id);
            cJSON_AddNumberToObject(item, "slot", global_placements[i].assigned_slot);
            cJSON_AddStringToObject(item, "room", global_placements[i].assigned_room);
            cJSON_AddStringToObject(item, "subject", s->subject);
            cJSON_AddStringToObject(item, "faculty", s->faculty);
            cJSON_AddStringToObject(item, "division", s->joint_divisions_count > 0 ? "Joint" : s->division);
            cJSON_AddNumberToObject(item, "duration", s->duration);
            cJSON_AddNumberToObject(item, "fixed_slot", s->fixed_slot);
            cJSON_AddNumberToObject(item, "student_count", s->student_count);
            cJSON_AddItemToArray(placements_arr, item);
        }
    }
    
    cJSON_AddItemToObject(resp, "placements", placements_arr);
    cJSON_AddItemToObject(resp, "unplaced", unplaced_arr);
    return resp;
}

static void handle_get_timetable(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    cJSON *resp = build_timetable_json();
    send_json(c, resp);
}

static void handle_post_generate(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    preflight_check(global_sessions, global_num_sessions, global_rooms, global_num_rooms, &global_faculty_map, global_placements);
    color_timetable(global_sessions, global_num_sessions, global_rooms, global_num_rooms, &global_faculty_map, ORDERING_SPREAD, global_placements);
    repair_timetable(global_sessions, global_num_sessions, global_rooms, global_num_rooms, &global_faculty_map, global_placements, 2000);
    
    for(int i=0; i<global_num_sessions; i++) {
        db_save_placement(&global_placements[i]);
    }
    
    cJSON *resp = build_timetable_json();
    send_json(c, resp);
}

static void handle_post_upload(struct mg_connection *c, struct mg_http_message *hm) {
    FILE *f = fopen("data/uploaded.json", "w");
    if(f) {
        fwrite(hm->body.buf, 1, hm->body.len, f);
        fclose(f);
        
        unlink("chromaslot.db");
        db_init("chromaslot.db");
        load_adversarial_dataset("data/uploaded.json");
        
        db_load_rooms(global_rooms, &global_num_rooms);
        
        hash_map_free(&global_faculty_map);
        hash_map_init(&global_faculty_map, 101);
        global_num_faculty = 0;
        db_load_faculty(global_faculty, &global_num_faculty);
        for(int i=0; i<global_num_faculty; i++) hash_map_put(&global_faculty_map, global_faculty[i].name, &global_faculty[i]);
        
        db_load_sessions(global_sessions, &global_num_sessions);
        split_into_batches(global_sessions, &global_num_sessions, global_rooms, global_num_rooms);
        
        for (int i = 0; i < global_num_sessions; i++) {
            strcpy(global_placements[i].session_id, global_sessions[i].id);
            global_placements[i].assigned_slot = -1;
            global_placements[i].reason_code = 0;
            global_placements[i].blocking_reason[0] = '\0';
            global_placements[i].assigned_room[0] = '\0';
        }
        
        mg_http_reply(c, 200, "Content-Type: application/json\r\nCache-Control: no-cache, no-store, must-revalidate\r\n", "{\"status\":\"ok\"}");
    } else {
        mg_http_reply(c, 500, "", "error");
    }
}

static void handle_get_health(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    HealthCheckReport r;
    run_health_check(global_sessions, global_num_sessions, global_rooms, global_num_rooms, &global_faculty_map, global_placements, &r);
    
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddNumberToObject(resp, "unplaced_sessions", r.unplaced_sessions);
    cJSON_AddNumberToObject(resp, "faculty_conflicts", r.faculty_conflicts);
    cJSON_AddNumberToObject(resp, "room_conflicts", r.room_conflicts);
    cJSON_AddNumberToObject(resp, "division_conflicts", r.division_conflicts);
    cJSON_AddNumberToObject(resp, "consecutive_violations", r.consecutive_violations);
    cJSON_AddNumberToObject(resp, "unavail_violations", r.unavail_violations);
    cJSON_AddNumberToObject(resp, "capacity_violations", r.capacity_violations);
    cJSON_AddNumberToObject(resp, "total_issues", r.total_issues);
    send_json(c, resp);
}

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        
        if (mg_match(hm->uri, mg_str("/api/health"), NULL)) {
            handle_get_health(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/generate"), NULL)) {
            handle_post_generate(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/timetable"), NULL)) {
            handle_get_timetable(c, hm);
        } else if (mg_match(hm->uri, mg_str("/api/upload"), NULL)) {
            handle_post_upload(c, hm);
        } else {
            struct mg_http_serve_opts opts = {.root_dir = "../frontend/dist"};
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}

int start_server(const char *port) {
    db_init("chromaslot.db");
    db_load_rooms(global_rooms, &global_num_rooms);
    hash_map_init(&global_faculty_map, 101);
    global_num_faculty = 0;
    db_load_faculty(global_faculty, &global_num_faculty);
    for(int i=0; i<global_num_faculty; i++) hash_map_put(&global_faculty_map, global_faculty[i].name, &global_faculty[i]);
    db_load_sessions(global_sessions, &global_num_sessions);
    split_into_batches(global_sessions, &global_num_sessions, global_rooms, global_num_rooms);
    
    for (int i = 0; i < global_num_sessions; i++) {
        strcpy(global_placements[i].session_id, global_sessions[i].id);
        global_placements[i].assigned_slot = -1;
    }
    db_load_placements(global_placements, global_num_sessions);
    
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    char listen_url[64];
    snprintf(listen_url, sizeof(listen_url), "http://0.0.0.0:%s", port);
    if (mg_http_listen(&mgr, listen_url, ev_handler, NULL) == NULL) return -1;
    while (1) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
    return 0;
}
