/*
 * Ring channels provide a general producer-consumer interface for relaying
 * data from to the LCD components. Specifically ring channels are used for
 * establishing a high-performance communication channels between LCD components
 *
 * Authors: Anton Burtsev, Scotty Bauer
 * Date:    October 2011,  Feburary 2015
 *
 */


#include <asm-generic/getorder.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include "ring-channel.h"


static inline unsigned long bsrl(unsigned long x)
{
	unsigned long ret;
	asm("bsr %1,%0" : "=r"(ret) : "r"(x));
	return ret;

}

static inline unsigned long lower_power_of_two(unsigned long x)
{
	return 0x80000000000000UL >>  (__builtin_clzl(x)-1);
}


/* Stolen from  xen/mm.h */
static inline int get_order_from_pages(unsigned long nr_pages)
{
	int order;
	nr_pages--;
	for ( order = 0; nr_pages; order++ )
		nr_pages >>= 1;
	return order;
}

int ttd_ring_channel_alloc(struct ttd_ring_channel *ring_channel,
			   unsigned long size_in_pages,
			   unsigned long size_of_a_rec) {
	return ttd_ring_channel_alloc_with_metadata(ring_channel,
						    size_in_pages, size_of_a_rec, 0);
}

void ttd_ring_channel_free(struct ttd_ring_channel *ring_channel) {

	unsigned long order;
	if (ring_channel->tx.recs) {
		order = get_order_from_pages(ring_channel->tx.size_in_pages);
		free_pages((unsigned long) ring_channel->tx.recs,
			   order);
		ring_channel->tx.recs = NULL;
	}
}

int ttd_ring_channel_alloc_with_metadata(struct ttd_ring_channel *ring_channel,
                                         unsigned long size_in_pages,
                                         unsigned long size_of_a_rec,
                                         unsigned long priv_metadata_size)
{
	int           ret;
	unsigned long order, header_order;
	unsigned long size_of_header_in_pages;
	pr_debug("Allocating ring channel\n");
	ttd_ring_channel_init(ring_channel);


	/* number of pages required for this */
	header_order = get_order(priv_metadata_size +
					    sizeof(struct ttd_buf));


	size_of_header_in_pages = 1 << header_order;
	pr_debug("Allocating ring channel: header area size:%lu, in pages:%lu\n",
		 priv_metadata_size + sizeof(struct ttd_buf), size_of_header_in_pages);

	order = get_order_from_pages(size_in_pages);
	if ( (ring_channel->tx.recs = (char *) __get_free_pages(GFP_KERNEL, order)) == NULL ) {
		pr_err("Xen deterministic time-travel buffers: memory allocationcd failed, "
		       "size in pages:%lu, order:%lu\n", size_in_pages, order);
		ret = -EINVAL; goto cleanup;
	}

	ring_channel->tx.slot = 0;

	ring_channel->tx.size_of_a_rec = size_of_a_rec;
	pr_debug("Size of a rec is %lu\n", size_of_a_rec);


	ring_channel->tx.order_two_mask = (size_in_pages * PAGE_SIZE)-1;
	ring_channel->tx.size_in_pages = size_in_pages;


	return 0;

 cleanup:

	ttd_ring_channel_free(ring_channel);
	return ret;

}

