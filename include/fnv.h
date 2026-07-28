/**
 * @file fnv.h
 * @brief Implementation for the FNV-1A hash function. Ref.:https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_f
 * @ingroup helper
 *
*/

#ifndef FNV_H
#define FNV_H

#include <stdint.h>
#include <stdlib.h>

#define FNV_PRIME UINT64_C(1099511628211)
#define FNV_OFFSET_BASIS UINT64_C(14695981039346656037)

/**
 * Generates 64-byte FNV-1a hash for given input.
 *
 * @param input Pointer to input data
 * @param size Size of the input data.
 *
 * @warning Do not pass sizeof(input) as size. Requires sizeof the actual input data.
 *
 * @return Calculated FNV-1a hash of the input.
 *
*/
uint64_t fnv(void* input, size_t size);

#endif
