/**
 * @file hmap.h
 * @brief Hash Map implementation for the compiler.
 * @ingroup helper
 *
 */

#ifndef HMAP_H
#define HMAP_H

#include "fnv.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * Hash function used for the table.
 */
#define HASH(input, size) fnv(input, size)
/**
 * Initial capacity for the hash map.
 * @warning The initial capacity must be a power of 2.
 */
#define INIT_CAPACITY 8
/**
 * Maxmimum load permitted before re-allocation.
 */
#define MAX_LOAD 0.5

/**
 * Hash Map data structure.
 *
 * @note Currently keys are only strings for the compiler-implementation.
 */
struct HashMap{
	/** Internal storage array for hash entries. Do not access directly. **/
	struct HashEntry** arr;
	/** Current maximum capcaity of the hashmap. **/
	size_t capacity;
	/** Number of entries present in the hashmap. **/
	size_t entries;
};

/**
 * Initializes a hash map data structure.
 *
 * @return Pointer to heap allocated hashmap.
 */
struct HashMap* hmap_init(void);

/**
 *
 * Get value for a given key in the hashmap.
 *
 * @param hmap Hash map data structure.
 * @param key Key for searching the value.
 *
 * @return Pointer to the value, returns NULL if not found.
 */
void* hmap_get(struct HashMap* hmap, char* key);

/**
 * Delete a key from the hashmap.
 *
 * @param hmap Hash map data structure.
 * @param key Key to be deleted.
 *
 * @return Pointer to the deleted value, NULL if not found.
 *
 * @note Does not free the said values, ownership belongs to the caller.
 */
void* hmap_del(struct HashMap* hmap, char* key);

/**
 * Add a key-value pair to the hashmap.
 *
 * @param hmap Hash map data structure.
 * @param key Key to be inserted.
 * @param val Pointer to value to be inserted.
 *
 * @return Incase a value exists already for given key, returns the replaced value. Otherwise NULL.
 */
void* hmap_add(struct HashMap* hmap, char* key, void* val);

#endif
