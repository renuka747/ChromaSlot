#include "hash_map.h"
#include <stdlib.h>
#include <string.h>

/*
 * djb2 hash function
 * Time: O(K) where K = length of string.
 */
static unsigned long djb2_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

int hash_map_init(HashMap *map, int num_buckets) {
    if (!map || num_buckets <= 0) return -1;
    map->num_buckets = num_buckets;
    map->buckets = calloc(num_buckets, sizeof(HashEntry*));
    if (!map->buckets) return -1;
    return 0;
}

void hash_map_free(HashMap *map) {
    if (!map || !map->buckets) return;
    for (int i = 0; i < map->num_buckets; i++) {
        HashEntry *entry = map->buckets[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    map->buckets = NULL;
    map->num_buckets = 0;
}

int hash_map_put(HashMap *map, const char *key, void *value) {
    if (!map || !map->buckets || !key) return -1;
    
    unsigned long hash = djb2_hash(key);
    int idx = hash % map->num_buckets;
    
    HashEntry *entry = map->buckets[idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return 0;
        }
        entry = entry->next;
    }
    
    entry = malloc(sizeof(HashEntry));
    if (!entry) return -1;
    
    strncpy(entry->key, key, MAX_ID_LEN - 1);
    entry->key[MAX_ID_LEN - 1] = '\0';
    entry->value = value;
    entry->next = map->buckets[idx];
    map->buckets[idx] = entry;
    
    return 0;
}

void* hash_map_get(const HashMap *map, const char *key) {
    if (!map || !map->buckets || !key) return NULL;
    
    unsigned long hash = djb2_hash(key);
    int idx = hash % map->num_buckets;
    
    HashEntry *entry = map->buckets[idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}
