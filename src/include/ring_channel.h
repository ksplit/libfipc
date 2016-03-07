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
#include <linux/types.h>
#include <linux/slab.h>
#include "ipc.h"

/* struct ttd_buf { */
/* 	/\* PRODUCER *\/ */
/* 	unsigned long      slot; */
/* 	unsigned long      size_of_a_rec;        /\* size of a single record *\/ */
/* 	unsigned long      order_two_mask; */
/* 	unsigned long      size_in_pages; */
/* 	char               *recs;                 /\* pointer to buffer data areas      *\/ */
/* 	uint8_t             padding[16]; /\* pad the struct up to cache line size *\/ */
/* }; */

//from ipc.h
/* struct ipc_message; */

/* struct ttd_ring_channel { */
/* 	struct ttd_buf tx; */
/* 	struct ttd_buf rx; */
/* 	/\* TODO NECESSARY? *\/ */
/*     int (*dispatch_fn)(struct ttd_ring_channel*, struct ipc_message*); */
/* 	uint8_t padding[56]; /\* pad the struct to cacheline size *\/ */
/* }; */

/* struct ttd_ring_channel_group */
/* { */
/*     struct ttd_ring_channel **chans; */
/*     size_t chans_length; */
/*     struct task_struct *thread; */
/* }; */


/* static inline void channel_group_alloc(struct ttd_ring_channel_group* channel_group, size_t chans_length) */
/* { */
/*     struct ttd_ring_channel **chans_arr = (struct ttd_ring_channel **)kzalloc( */
/*                                     sizeof(struct ttd_ring_channel*)*chans_length,  */
/*                                     GFP_KERNEL); */
/*     if( !chans_arr ) */
/*     { */
/*         pr_err("could not allocate memory for ring channel group\n"); */
/*         return; */
/*     } */
/*     channel_group->chans        = chans_arr; */
/*     channel_group->chans_length = chans_length; */
/* } */


/* static inline void channel_group_free(struct ttd_ring_channel_group* channel_group) */
/* { */
/*     kfree(channel_group->chans); */
/* } */


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
  We have to const the rec_size because for some reason I cannot put
  sizeof (struct IPC_message) gcc will _usually_ constant fold this
  since it realizes rec_size is const and is available at compile time
*/
static inline void *get_tx_rec(struct ttd_ring_channel *rc,
			      const unsigned long rec_size)
{
	//	pr_err("math for tx & comes out to be %lu, with slot %lu and mask %lx\n", (rc->tx.slot & rc->tx.order_two_mask), rc->tx.slot, rc->tx.order_two_mask);

	return (void*) (rc->tx.recs +
		((rc->tx.slot & rc->tx.order_two_mask) *
		 rec_size));
}


static inline void* get_rx_rec(struct ttd_ring_channel *rc,
			       const unsigned long rec_size)
{
	//	pr_err("math for rx & comes out to be %lu, with slot %lu and mask %lx\n", rc->rx.slot & rc->rx.order_two_mask, rc->rx.slot, rc->tx.order_two_mask);
	return (void*) (rc->rx.recs +
			((rc->rx.slot & rc->rx.order_two_mask) *
			 rec_size));
}



#endif

