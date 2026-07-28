/**
 * @file stream.h
 * @brief Generic Stream implementation. Behaves akin to a FIFO queue.
 * @ingroup helper
 *
 * @todo Exploring ring buffers for stream implementation.
 * 
 */

#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define INITIAL_STREAM_SIZE 64

/**
 * Stream data structure. Implemented using an array of void*
 */
struct Stream{
	/** Internal stream array. Do not modify/access. **/
	void** stream;
	/** Index to beginning of stream. **/
	size_t begin;
	/** Number of elements in the stream. **/
	size_t length; 
	/** Maximum current capacity of the stream. **/
	size_t capacity;


};

/**
 * Stream Iterator, may be used as an array for manual access.
 *
 * @note After retrieval, stream iterator state is NOT synced with the stream, and as such is only valid upto the point of retrieval.
 */
struct StreamIterator{
	/** void* array containing stream elements. **/
	void** arr;
	/** Length of the array. **/
	size_t n;
};

/**
 * Initializes stream data structure with default parameters.
 *
 * @return Pointer to heap allocated stream data structure.
 */
struct Stream* stream_init(void);

/**
 * Append element to the stream.
 *
 * @param st Stream data structure
 * @param ele Pointer to be appended.
 *
 * @return true on successful append, false otherwise.
 */
bool stream_append(struct Stream* st, void* ele);

/**
 * Peek first element from the stream.
 *
 * @param st Stream data structure
 *
 * @return Pointer value of the element. NULL if empty.
 */
void* stream_peek(struct Stream* st);

/**
 * Peek second element from the stream
 *
 * @param st Stream data structure
 *
 * @return Pointer value of the element. NULL if second element not present.
 */ 
void* stream_peekNext(struct Stream* st);

/**
 * Advance stream forward (First element is discarded)
 *
 * @param st Stream data structure
 * 
 * @return Pointer value of the discard element. NULL if empty.
 *
 * @note Ownership of elements belongs to the caller. No attempts to free() are made.
 */
void* stream_advance(struct Stream* st);

/**
 * Compact the stream size by shrinking un-used memmory spaces.
 *
 * @param st Stream data structure.
 * 
 * @return true on success, false otherwise.
 */
bool stream_compact(struct Stream* st);

/**
 * Destroy and free the entire stream.
 * 
 * @param st Stream data structure
 *
 * @note Ownership of elements belongs to the caller. No attempts to free() are made.
 */
void stream_destroy(struct Stream* st);

/**
 * Check if the stream is empty.
 *
 * @param st Stream data structure.
 *
 * @return true if empty, false otherwise
 */
bool stream_isEmpty(struct Stream* st);

/**
 * Returns a StreamIterator of the current stream state.
 *
 * @param st Stream data structure
 *
 * @return StreamIterator of the current stream state.
 */
struct StreamIterator stream_getIterator(struct Stream* st);
#endif
