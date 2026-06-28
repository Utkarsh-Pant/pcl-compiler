/*
 * Some ideas: 
 * String interning (q_q)
 * Tracking no. of deadspots and resizing based on that.
 * For double safety, check for duplicates in hmap_add_no_realloc too? (Imo not necessary, but keep in mind)
 *
 *
 */


#include "hmap.h"

struct HashMap* hmap_init(void){
	struct HashMap* new = malloc(sizeof(struct HashMap));
	new->arr = calloc(INIT_CAPACITY, sizeof(struct HashEntry*));
	new->capacity = INIT_CAPACITY;
	new->entries = 0;

	return new;
}

static void hmap_add_no_realloc(char* key, struct HashEntry* val_entry, struct HashMap* hmap){


	uint64_t hsh = HASH(key, strlen(key)+1); // +1 to include \0
	size_t loc = hsh & (hmap->capacity-1);
	

	int first_seen = -1;
	for(size_t i = 0; i<hmap->capacity; i++){
		loc = (hsh + (i*(i+1)/2)) & (hmap->capacity-1);
		if(hmap->arr[loc] == NULL) break; // INSERT >:)
		else if(hmap->arr[loc] == BK_DEL) 
			if(first_seen == -1) first_seen = loc;
		
		// No need to check for already existing, since this is only used 
		// during resizing, and so will never be called twice on the same key : )

		/*else if(
			hmap->arr[loc]->hsh == hsh &&
			strcmp(hmap->arr[loc]->key, key) == 0
		){
			hmap->arr[loc]->val = val;
			return;
		}*/


	}

	if(first_seen != -1) loc = first_seen; 

	hmap->arr[loc] = val_entry;
	
}


static void resize(struct HashMap* hmap){
	if( (((float)hmap->entries)/hmap->capacity) <= MAX_LOAD) return;
		
	size_t old_cap = hmap->capacity;
	hmap->capacity = 2*hmap->capacity;
	struct HashEntry** old_arr = hmap->arr;
	hmap->arr = calloc(hmap->capacity, sizeof(struct HashEntry*));

	for(size_t i = 0; i<old_cap; i++){
		if(old_arr[i]!= NULL && old_arr[i] != BK_DEL){
			hmap_add_no_realloc(old_arr[i]->key, old_arr[i], hmap);
		}
	}

	free(old_arr);
}

void* hmap_get(char* key, struct HashMap* hmap){
	uint64_t hsh = HASH(key, strlen(key)+1);
	size_t loc = hsh & (hmap->capacity-1);
		
	for(size_t i = 0; i<hmap->capacity; i++){
		loc = (hsh + (i*(i+1)/2)) & (hmap->capacity-1);
		if(hmap->arr[loc] == NULL) return NULL;
		else if(
			hmap->arr[loc] != BK_DEL && 
			hmap->arr[loc]->hsh == hsh &&
			strcmp(hmap->arr[loc]->key, key) == 0
		)return hmap->arr[loc]->val;
			
	}


	return NULL;
};

void* hmap_del(char* key, struct HashMap* hmap){
	
	uint64_t hsh = HASH(key, strlen(key)+1);
	size_t loc = hsh & (hmap->capacity-1);
		
	for(size_t i = 0; i<hmap->capacity; i++){
		loc = (hsh + (i*(i+1)/2)) & (hmap->capacity-1);
		if(hmap->arr[loc] == NULL) return NULL;
		else if(
			hmap->arr[loc] != BK_DEL && 
			hmap->arr[loc]->hsh == hsh &&
			strcmp(hmap->arr[loc]->key, key) == 0
		){
			void* ret_val = hmap->arr[loc]->val;	
			free(hmap->arr[loc]->key);
			free(hmap->arr[loc]);
			
			hmap->arr[loc] = BK_DEL;
			hmap->entries--;

			return ret_val;
		
		}	
	}


	return NULL;
}

void hmap_add(char* key, void* val, struct HashMap* hmap){


	uint64_t hsh = HASH(key, strlen(key)+1); // +1 to include \0
	size_t loc = hsh & (hmap->capacity-1);
	

	int first_seen = -1;
	for(size_t i = 0; i<hmap->capacity; i++){
		loc = (hsh + (i*(i+1)/2)) & (hmap->capacity-1);
		if(hmap->arr[loc] == NULL) break; // INSERT >:)
		else if(hmap->arr[loc] == BK_DEL) 
			if(first_seen == -1) first_seen = loc;
		else if(
			hmap->arr[loc]->hsh == hsh &&
			strcmp(hmap->arr[loc]->key, key) == 0
		){
			hmap->arr[loc]->val = val;
			return;
		}


	}

	if(first_seen != -1) loc = first_seen; 

	struct HashEntry* new_entry = malloc(sizeof(struct HashEntry));
	new_entry->hsh = hsh;
	new_entry->key = strdup(key);
	new_entry->val = val;

	hmap->arr[loc] = new_entry;
	hmap->entries++;
	resize(hmap);
	
}

