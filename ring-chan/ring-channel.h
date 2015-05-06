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
#include <../IPC/IPC.h>

struct ttd_buf {
	/* PRODUCER */
	unsigned long      slot;
	unsigned long      size_of_a_rec;        /* size of a single record */
	unsigned long      order_two_mask;
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


static inline unsigned long get_tx_slot(struct ttd_ring_channel *rc)
{
	return rc->tx.slot;
}

static inline unsigned long get_rx_slot(struct ttd_ring_channel *rc)
{
	return rc->rx.slot;
}

static inline unsigned long inc_tx_slot(struct ttd_ring_channel *rc)
{
	return (rc->tx.slot++);
}

static inline unsigned long inc_rx_slot(struct ttd_ring_channel *rc)
{
	return (rc->rx.slot++);
}

static inline void set_tx_slot(struct ttd_ring_channel *rc, unsigned long num)
{
	rc->tx.slot = num;
}

static inline void set_rx_slot(struct ttd_ring_channel *rc, unsigned long num)
{
	rc->rx.slot = num;
}

/*
  If we can set a constant buffer size we can constant fold more.
*/
static inline char* tx_get_rec(struct ttd_ring_channel *rc)
{
	return (rc->tx.recs +
		((rc->tx.slot & rc->tx.order_two_mask) *
		 sizeof(struct ipc_message)));
}

static inline char* tx_get_rec(struct ttd_ring_channel *rc)
{
	return (rc->tx.recs +
		((rc->tx.slot & rc->tx.order_two_mask) *
		 sizeof(struct ipc_message)));
}

static inline char* rx_get_rec(struct ttd_ring_channel *rc)
{
	return (rc->rx.recs +
		((rc->rx.slot & rc->rx.order_two_mask) *
		 sizeof(struct ipc_message)));
}



#endif

