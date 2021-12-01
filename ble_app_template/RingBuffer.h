/*
 * ======== RingBuffer.h ========
 *
 *  Created on: July 25, 2017
 *  Author: Michael Moon -> Amber Agriculture, Inc.
 *  Description: Ring Buffer
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * RingBuffer logic:
 *
 * This is written as a C implementation of a C++-style templated class.
 *
 * These methods are inlined in the header so they can be more versatile than macros but still gain the various advantages of inlined code.
 *
 * All arithmetic is modulo 'length' - head or tail >= length is an illegal value
 *
 * head == tail means buffer empty.
 *
 * Unfortunately that means that the "full" capacity is length-1, ie there's always at least one unused item.
 *
 * It can be being filled by the provider, but the actual push can't happen until one item is popped
 *
 * Pushing to a buffer where head+1 == tail will drop all packets in the buffer - please check CanPush before every push!
 *
 * Popping from a buffer where head == tail will re-fill all packets in the buffer - please check CanPop before every pop!
 *
 * These checks have been omitted from the methods herein since application code can do more interesting things when the calling context is known, and adding them here would thus be a reduction in efficiency.
 *
 * Rinbuffers are inherently threadsafe as long as writing to head/tail is atomic. The struct is *not* packed for this reason, and the head/tail are marked volatile to force the compiler to access them appropriately.
 *
 * The Used and Free methods are provided as a debugging convenience but should not be relied upon for buffer management since they can spit out bogus numbers during pointer update race conditions.
 *
 * The CanPush and CanPop methods should be used for all buffer management decisions, as even during a race condition they can only return a stale value rather than a destructively incorrect one
 *
 * There is no need for the RingBuffer class to have any knowledge of the buffer location, or the size of the items within it - this "class" is simply index management.
 *
 * The buffer could contain chars, ints, structs, whatever.
 *
 * Simply write to buffer[rb->head] then push to provide, and read from buffer[rb->tail] then pop to consume
 */

// RingBuffer<uint8_t> "class"

typedef struct {
	volatile uint8_t head;
	volatile uint8_t tail;
	uint8_t length;
} RingBuffer8_t;

inline uint8_t RB8_NextHead(RingBuffer8_t* this) {
	if ((this->head + 1) >= this->length)
		return 0;
	return this->head + 1;
}

inline uint8_t RB8_NextTail(RingBuffer8_t* this) {
	if ((this->tail + 1) >= this->length)
		return 0;
	return this->tail + 1;
}

inline int RB8_CanPush(RingBuffer8_t* this) {
	if (RB8_NextHead(this) != this->tail)
		return 1;
	return 0;
}

inline int RB8_CanPop(RingBuffer8_t* this) {
	if (this->tail != this->head)
		return 1;
	return 0;
}

inline void RB8_Push(RingBuffer8_t* this) {
	this->head = RB8_NextHead(this);
}

inline void RB8_Pop(RingBuffer8_t* this) {
	this->tail = RB8_NextTail(this);
}

inline uint8_t RB8_Used(RingBuffer8_t* this) {
	if (this->head >= this->tail)
		return this->head - this->tail;
	return this->head + this->length - this->tail;
}

inline uint8_t RB8_Free(RingBuffer8_t* this) {
	if (this->tail > this->head)
		return this->tail - this->head;
	return this->tail + this->length - this->head;
}

// RingBuffer<uint16_t> "class"

typedef struct {
	volatile uint16_t head;
	volatile uint16_t tail;
	uint16_t length;
} RingBuffer16_t;

inline uint16_t RB16_NextHead(RingBuffer16_t* this) {
	if ((this->head + 1) >= this->length)
		return 0;
	return this->head + 1;
}

inline uint16_t RB16_NextTail(RingBuffer16_t* this) {
	if ((this->tail + 1) >= this->length)
		return 0;
	return this->tail + 1;
}

inline int RB16_CanPush(RingBuffer16_t* this) {
	if (RB16_NextHead(this) != this->tail)
		return 1;
	return 0;
}

inline int RB16_CanPop(RingBuffer16_t* this) {
	if (this->tail != this->head)
		return 1;
	return 0;
}

inline void RB16_Push(RingBuffer16_t* this) {
	this->head = RB16_NextHead(this);
}

inline void RB16_Pop(RingBuffer16_t* this) {
	this->tail = RB16_NextTail(this);
}

inline uint16_t RB16_Used(RingBuffer16_t* this) {
	if (this->head >= this->tail)
		return this->head - this->tail;
	return this->head + this->length - this->tail;
}

inline uint16_t RB16_Free(RingBuffer16_t* this) {
	if (this->tail > this->head)
		return this->tail - this->head;
	return this->tail + this->length - this->head;
}

#endif /* RINGBUFFER_H */
