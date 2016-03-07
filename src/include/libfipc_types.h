/*
 * libfipc_types.h
 *
 * Struct definitions, etc. for libfipc library.
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_TYPES_H
#define LIBFIPC_TYPES_H

/**
 * struct fipc_message
 *
 * This is the data in each slot in an IPC ring buffer. It should fit
 * into one cache line. All fields are available for use, except
 * msg_status - this is reserved and is used internally by libfipc
 * to track the status of individual message slots in the IPC ring buffer.
 *
 * XXX: This probably needs to be arch-specific in order to fit in a
 * cache line.
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
 * it to be cacheline-aligned.
 */ 
struct fipc_ring_buf {
	/**
	 * Where *I* am in the IPC buffer. (The other guy knows where I am
	 * by looking at message statuses.)
	 */
	unsigned long      slot;
	/**
	 * IPC ring buffer is 2^nr_pages_order pages
	 */
	unsigned long      nr_pages_order;
	/**
	 * This is pre-computed so that we can quickly calculate the
	 * message slot index for slot allocations. It is given by:
	 *
	 *    [2^nr_pages_order * PAGE_SIZE / sizeof(struct fipc_message)] - 1
	 *
	 * Notice that because struct fipc_message and PAGE_SIZE are powers
	 * of 2, this mask will be 2^x - 1 for some x.
	 */
	unsigned long      order_two_mask;
	/**
	 * Pointer to the actual IPC buffer
	 */
	char               *recs;
	/**
	 * Pad the struct up to cacheline size
	 */
	uint8_t             padding[32];
};

/**
 * struct fipc_ring_channel
 *
 * A full duplex IPC channel, made up of two, one-way IPC ring buffers,
 * @tx and @rx.
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
