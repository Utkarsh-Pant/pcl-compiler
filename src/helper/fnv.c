#include "fnv.h"

/*
 * Input: Pointer to hash-input
 * Size: sizeof(hash-input)
 *
 */
uint64_t fnv(void* input, size_t size){
	uint64_t hash = FNV_OFFSET_BASIS;
	
	for(size_t i = 0; i<size; i++){
		unsigned char byte = ((unsigned char*) input)[i]; // Iterate byte-wise
		hash ^= byte;
		hash *= FNV_PRIME;
	}

	return hash;

}
