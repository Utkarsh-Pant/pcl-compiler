#include "fnv.h"
#include <stdio.h>
#include <string.h>

#ifndef HMAP_H
#define HMAP_H

#define HASH(input, size) fnv(input, size)
#define INIT_CAPACITY 8 // MUST BE POWER OF 2.
#define MAX_LOAD 0.5

#define BK_DEL (struct HashEntry*)1

struct HashEntry{
	uint64_t hsh;
	char* key;
	void* val;
};

struct HashMap{
	struct HashEntry** arr;
	size_t capacity;
	size_t entries;
};

struct HashMap* hmap_init(void);
void* hmap_get(char* key, struct HashMap* hmap);
void* hmap_del(char* key, struct HashMap* hmap);
void hmap_add(char* key, void* val, struct HashMap* hmap);

#endif
