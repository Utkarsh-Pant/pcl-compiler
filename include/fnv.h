/*
 * My implementation for the FNV-1a hash function
 * (Hopefully not broken... maybe?)
 *
 * Ref.: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash
 */

#include <stdint.h>
#include <stdlib.h>

#ifndef FNV_H
#define FNV_H

#define FNV_PRIME UINT64_C(1099511628211)
#define FNV_OFFSET_BASIS UINT64_C(14695981039346656037)

uint64_t fnv(void* input, size_t size);

#endif
