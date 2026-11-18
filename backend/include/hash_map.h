/*
 * hash_map.h — Generic string-keyed hash map using djb2
 */
#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "constants.h"

typedef struct HashEntry {
    char key[MAX_ID_LEN];
    void *value;
    struct HashEntry *next;
} HashEntry;

typedef struct {
    HashEntry **buckets;
    int num_buckets;
} HashMap;

/*
 * hash_map_init() - Initializes the hash map with the given number of buckets.
 * Returns 0 on success, -1 on failure.
 * Time: O(B) where B = num_buckets.
 */
int hash_map_init(HashMap *map, int num_buckets);

/*
 * hash_map_free() - Frees the hash map's buckets and entries. 
 * Note: Does not free the `value` pointers themselves.
 * Time: O(N) where N = number of elements.
 */
void hash_map_free(HashMap *map);

/*
 * hash_map_put() - Inserts or updates a key-value pair.
 * Returns 0 on success, -1 on allocation failure.
 * Time: O(L) where L is bucket chain length. Expected O(1).
 */
int hash_map_put(HashMap *map, const char *key, void *value);

/*
 * hash_map_get() - Retrieves a value by key.
 * Returns pointer to value, or NULL if not found.
 * Time: O(L). Expected O(1).
 */
void* hash_map_get(const HashMap *map, const char *key);

#endif /* HASH_MAP_H */
