#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef STREAM_H
#define STREAM_H

#define INITIAL_STREAM_SIZE 64

// TODO: While implementing this i ran into some fancy terms:
// Ring buffer
// Stream views
// ...
// learn about them later

struct Stream{
	void** stream;
	size_t begin;
	size_t length; 
	size_t capacity;


};

// Iterable Stream object, may be used like an array.
struct StreamIterator{
	void** arr;
	size_t n;
};

struct Stream* createStream(void);
int appendStream(struct Stream*, void*);
void* peekStream(struct Stream*);
void* advanceStream(struct Stream*);
int compactStream(struct Stream*);
void destroyStream(struct Stream*);
int isEmptyStream(struct Stream*);

struct StreamIterator getIterator(struct Stream*);
#endif
