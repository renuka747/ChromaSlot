#include "json_loader.h"
#include "cJSON/cJSON.h"
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_adversarial_dataset(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *data = malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);
    
    cJSON *json = cJSON_Parse(data);
    free(data);
    if (!json) return -1;
    
    cJSON *rooms = cJSON_GetObjectItem(json, "rooms");
    if (cJSON_IsArray(rooms)) {
        cJSON *r;
        cJSON_ArrayForEach(r, rooms) {
            Room room; memset(&room, 0, sizeof(Room));
            cJSON *id = cJSON_GetObjectItem(r, "id");
            if (!id) id = cJSON_GetObjectItem(r, "name");
            if (id && id->valuestring) strncpy(room.id, id->valuestring, MAX_ID_LEN-1);
            
            cJSON *cap = cJSON_GetObjectItem(r, "capacity");
            if (cap) room.capacity = cap->valueint;
            
            cJSON *type = cJSON_GetObjectItem(r, "type");
            if (type) room.type = type->valueint;
            
            db_save_room(&room);
        }
    }
    
    cJSON *faculties = cJSON_GetObjectItem(json, "faculty");
    if (cJSON_IsArray(faculties)) {
        cJSON *fac;
        cJSON_ArrayForEach(fac, faculties) {
            Faculty f_struct; memset(&f_struct, 0, sizeof(Faculty));
            cJSON *nm = cJSON_GetObjectItem(fac, "name");
            if (nm && nm->valuestring) strncpy(f_struct.name, nm->valuestring, MAX_NAME_LEN-1);
            
            cJSON *mc = cJSON_GetObjectItem(fac, "max_consecutive");
            if (mc) f_struct.max_consecutive = mc->valueint;
            
            cJSON *mask = cJSON_GetObjectItem(fac, "unavail_mask");
            if (mask) {
                f_struct.unavail_mask = (uint64_t)mask->valuedouble;
            } else {
                cJSON *arr = cJSON_GetObjectItem(fac, "unavail");
                if (cJSON_IsArray(arr)) {
                    uint64_t m = 0;
                    cJSON *slot;
                    cJSON_ArrayForEach(slot, arr) {
                        if (slot->valueint >= 0 && slot->valueint < 35) {
                            m |= (1ULL << slot->valueint);
                        }
                    }
                    f_struct.unavail_mask = m;
                }
            }
            db_save_faculty(&f_struct);
        }
    }
    
    cJSON *divisions = cJSON_GetObjectItem(json, "divisions");
    if (cJSON_IsArray(divisions)) {
        cJSON *d;
        cJSON_ArrayForEach(d, divisions) {
            Division div; memset(&div, 0, sizeof(Division));
            cJSON *nm = cJSON_GetObjectItem(d, "name");
            if (nm && nm->valuestring) strncpy(div.name, nm->valuestring, MAX_NAME_LEN-1);
            
            cJSON *sz = cJSON_GetObjectItem(d, "size");
            if (!sz) sz = cJSON_GetObjectItem(d, "student_count");
            if (sz) div.size = sz->valueint;
            
            db_save_division(&div);
        }
    }
    
    cJSON *sessions = cJSON_GetObjectItem(json, "sessions");
    if (cJSON_IsArray(sessions)) {
        cJSON *s;
        cJSON_ArrayForEach(s, sessions) {
            Session sess; session_init(&sess);
            
            cJSON *sid = cJSON_GetObjectItem(s, "id");
            if (sid && sid->valuestring) strncpy(sess.id, sid->valuestring, MAX_ID_LEN-1);
            
            cJSON *subj = cJSON_GetObjectItem(s, "subject");
            if (subj && subj->valuestring) strncpy(sess.subject, subj->valuestring, MAX_NAME_LEN-1);
            
            cJSON *fac = cJSON_GetObjectItem(s, "faculty");
            if (fac && fac->valuestring) strncpy(sess.faculty, fac->valuestring, MAX_NAME_LEN-1);
            
            cJSON *divi = cJSON_GetObjectItem(s, "division");
            if (divi && divi->valuestring) strncpy(sess.division, divi->valuestring, MAX_NAME_LEN-1);
            
            cJSON *dur = cJSON_GetObjectItem(s, "duration");
            if (dur) sess.duration = dur->valueint;
            
            cJSON *req = cJSON_GetObjectItem(s, "required_type");
            if (req) sess.required_type = req->valueint;
            
            cJSON *stu = cJSON_GetObjectItem(s, "student_count");
            if (stu) sess.student_count = stu->valueint;
            
            cJSON *split = cJSON_GetObjectItem(s, "splittable");
            if (split) sess.splittable = split->valueint;
            
            cJSON *fix = cJSON_GetObjectItem(s, "fixed_slot");
            if (fix) sess.fixed_slot = fix->valueint;
            
            cJSON *jd = cJSON_GetObjectItem(s, "joint_divisions");
            if (cJSON_IsArray(jd)) {
                cJSON *j;
                cJSON_ArrayForEach(j, jd) {
                    if (sess.joint_divisions_count < 5 && j->valuestring) {
                        strncpy(sess.joint_divisions[sess.joint_divisions_count++], j->valuestring, MAX_NAME_LEN-1);
                    }
                }
            }

            db_save_session(&sess);
        }
    }
    
    cJSON_Delete(json);
    return 0;
}
