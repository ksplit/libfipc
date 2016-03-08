/*
 * libfipc_types.h
 *
 * Struct definitions, etc. for libfipc library.
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_TYPES_H
#define LIBFIPC_TYPES_H

#include <libfipc_platform_types.h>

/**
 * Assumed cacheline size, in bytes.
 */
#define FIPC_CACHE_LINE_SIZE 64

/**
 * struct fipc_message
 *
 * This is the data in each slot in an IPC ring buffer. It should fit
 * into one cache line. All fields are available for use, except
 * msg_status - this is reserved and is used internally by libfipc
 * to track the status of individual message slots in the IPC ring buffer.
 *
 * XXX: This probably needs to be arch-specific in order to fit in a
 * cache line, and to ensure that msg_status can be updated atomically.
 *
 * XXX: The size of the message must be a power of 2.
 */
#define FIPC_NR_REGS 7
struct fipc_message {
	/**
	 * Reserved. Used internally to track message status.
	 */
	volatile uint32_t msg_status;
	/**
	 * Not touched by libfipc.
	 */
	uint32_t flags;
	/**
	 * Not touched by libfipc.
	 */
	unsigned long regs[FIPC_NR_REGS];
};

/**
 * struct fipc_ring_buf
 *
 * This is the header (metadata) for an IPC ring buffer (circular 
 * buffer). Each IPC ring buffer has a producer and consumer; each
 * will maintain its own struct fipc_ring_buf header. (Producers and consumers
 * communicate "where they are" in the IPC buffer using a special message
 * status field in the message slots in the buffer; see struct fipc_message
 * above.)
 *
 * XXX: This definition may need to be arch-specific in general if we want
 * it to be double cacheline sized.
 */
#define FIPC_RING_BUF_PADDING \
	(2 * FIPC_CACHE_LINE_SIZE - \
		(3 * sizeof(unsigned long) + sizeof(fipc_mutex_t)))
struct fipc_ring_buf {
	/**
	 * Protects slot. (Note that this synchronizes threads only
	 * on one side of a buffer.)
	 */
	fipc_mutex_t lock;
	/**
	 * Where *I* am in the IPC buffer. (The other guy knows where I am
	 * by looking at message statuses.)
	 */
	unsigned long slot;
	/**
	 * This is pre-computed at ring buffer initialization time so that 
	 * we can quickly calculate the message slot index, 
	 * circular-buffer style.
	 *
	 * Assuming the buffer itself is 2^buf_order bytes, the mask is given
	 * by:
	 *
	 *    [2^buf_order / sizeof(struct fipc_message)] - 1
	 *
	 * Since we require that struct fipc_message is a power of 2,
	 * this mask will be 2^x - 1 for some x.
	 */
	unsigned long order_two_mask;
	/**
	 * Pointer to the actual IPC buffer
	 */
	struct fipc_message *buffer;
	/**
	 * Pad the struct up to a cacheline
	 */
	uint8_t padding[FIPC_RING_BUF_PADDING];
};

/**
 * struct fipc_ring_channel
 *
 * A full duplex IPC channel, made up of two, one-way IPC ring buffers,
 * @tx and @rx.
 *
 * Note: It may seem redundant to store order_two_mask in both ring
 * buffers, rather than putting a common value here. We put redundant
 * values so that e.g. core A can send on the tx buffer while core B can
 * receive on the rx buffer, without any cacheline sharing.
 */
struct fipc_ring_channel {
	/**
	 * Pointer to our sending ring buffer.
	 */
	struct fipc_ring_buf tx;
	/**
	 * Pointer to our receiving ring buffer.
	 */
	struct fipc_ring_buf rx;
};

#endif /* LIBFIPC_TYPES_H */
