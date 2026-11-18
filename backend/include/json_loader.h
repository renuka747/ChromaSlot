/*
 * json_loader.h — Load JSON datasets.
 */
#ifndef JSON_LOADER_H
#define JSON_LOADER_H

/*
 * load_adversarial_dataset()
 * Reads JSON file using cJSON and writes into the SQLite DB via db_save_* functions.
 * Time: O(JSON_SIZE).
 * Returns 0 on success, -1 on failure.
 */
int load_adversarial_dataset(const char *filepath);

#endif
