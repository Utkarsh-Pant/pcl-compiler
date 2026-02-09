#include "stream.h"


struct Stream* createStream(){
	struct Stream* new_stream = malloc(sizeof(struct Stream));

	new_stream->stream = calloc(INITIAL_STREAM_SIZE, sizeof(void*));
	new_stream->capacity = INITIAL_STREAM_SIZE;
	new_stream->begin = 0;
	new_stream->length = 0;

	return new_stream;
}

int appendStream(struct Stream* st, void* ele){
	if(st->begin + st->length >= st->capacity){
		if(st->begin != 0){
			/* Shift Back */
			memmove(
				st->stream, 
				st->stream + st->begin, 
				st->length * sizeof(void*
			));
			st->begin = 0;	
		}
		else{
		/* Grow Stream */
			st->capacity = 2*st->capacity;
			st->stream = reallocarray(
					st->stream, 
					st->capacity, 
					sizeof(void*
					));
		}
	}

	st->stream[st->begin + (st->length)++] = ele;
	return EXIT_SUCCESS;
}

void* peekStream(struct Stream* st){
	if(st->length==0)return NULL;
	else return st->stream[st->begin];
}

void* advanceStream(struct Stream* st){
	if(st->length==0)return NULL;
	st->length--;
	return st->stream[st->begin++];
}

int compactStream(struct Stream* st){
	void** temp = calloc(st->length, sizeof(void*));
	memcpy(temp, st->stream + st->begin, st->length*sizeof(void*));
	free(st->stream);
	st->stream = temp;
	st->capacity = st->length;
	st->begin = 0;
	
	return EXIT_SUCCESS;	
}

void destroyStream(struct Stream* st){
	free(st->stream);
	free(st);
	
}

struct StreamIterator getIterator(struct Stream* st){
	struct StreamIterator itr= {
	.arr = st->stream + st->begin,
	.n = st->length
	};
	return itr;

}

