/*
 * common/ring-channel.c
 *
 * This file is part of the Flux deterministic time-travel infrastructure.
 * Ring channels provide a general producer-consumer interface for relaying
 * data from to the guest components. Specifically ring channels are used for
 * establishing a high-performance communication channels between Xen microkernel
 * and loggin, replay, and devd daemons.
 *
 * Authors: Anton Burtsev Scotty Bauer
 * Date:    October 2011 Feburary 2015
 *
 */

#ifndef __XEN_RING_CHANNEL_H__
#define __XEN_RING_CHANNEL_H__

#include <linux/string.h>


struct ttd_buf {
	/* PRODUCER */
	unsigned long      cons;      /* Next item to be consumed by control tools. */
	unsigned long      prod;      /* Next item to be produced by Xen.           */
	unsigned long      size_of_a_rec;        /* size of a single record */
	unsigned long      size_in_recs;         /* size of the buffer in recs */
	unsigned long      size_in_pages;
	char               *recs;                 /* pointer to buffer data areas      */
	uint_8             padding[16]; /* pad the struct up to cache line size */
}


struct ttd_ring_channel {
	struct ttd_buf tx_buf;
	struct ttd_buf rx_buf;
	struct task_struct *thread;
	/* TODO NECESSARY? */
	uint_8 padding[56]; /* pad the struct to cacheline size */
};


static inline void ttd_ring_channel_init(struct ttd_ring_channel *ring_channel)
{
	memset(ring_channel, 0, sizeof(*ring_channel));
	return;
}


int ttd_ring_channel_alloc(struct ttd_ring_channel *ring_channel,
			   unsigned long size_in_pages,
			   unsigned long size_of_a_rec);

int ttd_ring_channel_alloc_with_metadata(struct ttd_ring_channel *ring_channel,
					 unsigned long size_in_pages,
					 unsigned long size_of_a_rec,
					 unsigned long priv_metadata_size);

void ttd_ring_channel_free(struct ttd_ring_channel *ring_channel);

static inline void

static inline unsigned long
ttd_ring_channel_get_prod(struct ttd_ring_channel *ring_channel) {
	return ring_channel->buf->prod;
};

static inline unsigned long
ttd_ring_channel_inc_prod(struct ttd_ring_channel *ring_channel) {
	return (ring_channel->buf->prod++);
};

static inline void
ttd_ring_channel_set_prod(struct ttd_ring_channel *ring_channel, unsigned long prod) {
	ring_channel->buf->prod = prod;
	return;
};

static inline unsigned long
ttd_ring_channel_get_cons(struct ttd_ring_channel *ring_channel) {
	return ring_channel->buf->cons;
};

static inline unsigned long
ttd_ring_channel_inc_cons(struct ttd_ring_channel *ring_channel) {
	return (ring_channel->buf->cons++);
};


static inline void ttd_ring_channel_set_cons(struct ttd_ring_channel *ring_channel,
					     unsigned long cons) {
	ring_channel->buf->cons = cons;
	return;
};

static inline char *ttd_ring_channel_get_rec_slow(struct ttd_ring_channel *ring_channel,
						  unsigned long cons) {

	return (ring_channel->recs + (cons % 1024) * 64);
	//	return (ring_channel->recs
	//	+ (cons % ring_channel->size_in_recs)
	//	* ring_channel->size_of_a_rec);
};


static inline unsigned long ttd_ring_channel_get_index_mod_slow(struct ttd_ring_channel *ring_channel, unsigned long index) {
	return (index % ring_channel->size_in_recs);
}


static inline unsigned long ttd_ring_channel_size_in_recs(struct ttd_ring_channel *ring_channel) {
	return ring_channel->size_in_recs;
}

static inline unsigned long ttd_ring_channel_size_of_a_rec(struct ttd_ring_channel *ring_channel) {
	return ring_channel->size_of_a_rec;
}

static inline unsigned long ttd_ring_channel_size(struct ttd_ring_channel *ring_channel) {
	return ring_channel->size_in_recs * ring_channel->size_of_a_rec;
}


static inline unsigned long ttd_ring_channel_highwater(struct ttd_ring_channel *ring_channel) {
	return ring_channel->highwater;
}

static inline unsigned long ttd_ring_channel_emergency_margin(struct ttd_ring_channel *ring_channel) {
	return ring_channel->emergency_margin;
}

#endif

