/*
 * core_types.h — Central structs for ChromaSlot (Phase 1).
 */
#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include "constants.h"
#include <stdint.h>

typedef enum {
    ROOM_CLASSROOM = 0,
    ROOM_COMPUTER_LAB,
    ROOM_ELECTRONICS_LAB
} RoomType;

typedef enum {
    REASON_NONE = 0,
    REASON_CAPACITY,
    REASON_FACULTY_UNAVAIL,
    REASON_NO_ROOM,
    REASON_FIXED_SLOT_CLASH,
    REASON_NO_CONSECUTIVE_SLOTS,
    REASON_CONSECUTIVE_LIMIT,
    REASON_NO_FREE_SLOT
} UnplacedReason;

typedef struct {
    char id[MAX_ID_LEN];
    int capacity;
    RoomType type;
} Room;

typedef struct {
    char name[MAX_NAME_LEN];
    int max_consecutive;       /* 0 = unlimited */
    uint64_t unavail_mask;     /* Centralized unavailability */
} Faculty;

typedef struct {
    char name[MAX_NAME_LEN];
    int size;
} Division;

typedef struct {
    char id[MAX_ID_LEN];
    char subject[MAX_NAME_LEN];
    char faculty[MAX_NAME_LEN];
    char division[MAX_NAME_LEN];
    char room[MAX_NAME_LEN];   /* Legacy fallback */
    int fixed_slot;
    int unavail[MAX_UNAVAIL];
    int unavail_count;
    
    int student_count;
    int duration;
    char allowed_rooms[5][MAX_NAME_LEN];
    int allowed_rooms_count;
    char joint_divisions[5][MAX_NAME_LEN];
    int joint_divisions_count;
    RoomType required_type;
    char parent_id[MAX_ID_LEN];
    int batch_index;           /* 0 = not a batch */
    int splittable;            /* 1 = true, 0 = false */
} Session;

typedef struct {
    char session_id[MAX_ID_LEN];
    int assigned_slot;
    char assigned_room[MAX_NAME_LEN];
    UnplacedReason reason_code;
    char blocking_reason[MAX_REASON_LEN];
} SessionPlacement;

#endif /* CORE_TYPES_H */
