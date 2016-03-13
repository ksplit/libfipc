/*
 * ipc.c
 *
 * Authors: Anton Burtsev, Scotty Bauer
 * Date:    October 2011,  Feburary 2015
 *
 * Copyright: University of Utah
 */

#ifdef LCD_DOMAINS
#include <lcd_config/pre_hook.h>
#endif

#include <libfipc.h>
#include <libfipc_internal.h>

#ifdef LCD_DOMAINS
#include <lcd_config/post_hook.h>
#endif

#ifndef LINUX_KERNEL_MODULE
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(x)
#endif

#define FIPC_MSG_STATUS_AVAILABLE 0xdeaddeadUL
#define FIPC_MSG_STATUS_SENT      0xfeedfeedUL

static inline unsigned long get_tx_slot(struct fipc_ring_channel *rc)
{
	return rc->tx.slot;
}

static inline unsigned long get_rx_slot(struct fipc_ring_channel *rc)
{
	return rc->rx.slot;
}

static inline unsigned long get_tx_idx(struct fipc_ring_channel *rc)
{
	return rc->tx.slot & rc->tx.order_two_mask;
}

static inline unsigned long get_rx_idx(struct fipc_ring_channel *rc)
{
	return rc->rx.slot & rc->rx.order_two_mask;
}

static inline void set_tx_slot(struct fipc_ring_channel *rc, unsigned long num)
{
	rc->tx.slot = num;
}

static inline void set_rx_slot(struct fipc_ring_channel *rc, unsigned long num)
{
	rc->rx.slot = num;
}

static inline unsigned long inc_tx_slot(struct fipc_ring_channel *rc)
{
	return (rc->tx.slot++);
}

static inline unsigned long inc_rx_slot(struct fipc_ring_channel *rc)
{
	return (rc->rx.slot++);
}

static inline struct fipc_message* 
get_current_tx_slot(struct fipc_ring_channel *rc)
{
	return &rc->tx.buffer[get_tx_idx(rc)];
}

static inline struct fipc_message* 
get_current_rx_slot(struct fipc_ring_channel *rc)
{
	return &rc->rx.buffer[get_rx_idx(rc)];
}

static inline unsigned long
rx_msg_to_idx(struct fipc_ring_channel *rc, struct fipc_message *msg)
{
	return msg - rc->rx.buffer;
}

static inline unsigned long
tx_msg_to_idx(struct fipc_ring_channel *rc, struct fipc_message *msg)
{
	return msg - rc->tx.buffer;
}

static inline int check_rx_slot_msg_waiting(struct fipc_message *slot)
{
	return slot->msg_status == FIPC_MSG_STATUS_SENT;
}

static inline int check_tx_slot_available(struct fipc_message *slot)
{
	return slot->msg_status == FIPC_MSG_STATUS_AVAILABLE;
}

static inline unsigned long nr_slots(unsigned int buf_order)
{
	return (1UL << buf_order) / sizeof(struct fipc_ring_buf);
}

static inline unsigned long order_two_mask(unsigned int buf_order)
{
	return  nr_slots(buf_order) - 1;
}

int fipc_prep_buffers(unsigned int buf_order, void *buffer_1, void *buffer_2)
{
	unsigned long i;
	struct fipc_message *msg_buffer_1 = buffer_1;
	struct fipc_message *msg_buffer_2 = buffer_2;
	/*
	 * Buffers must be at least as big as one ipc message slot
	 */
	if ((1UL << buf_order) < sizeof(struct fipc_message)) {
		FIPC_DEBUG(FIPC_DEBUG_ERR,
			"Buffers are too small (buf_order = %d, so their size is 2^buf_order = %lu bytes); but one fipc message is %lu, so the buffers need to be at least that big\n",
			buf_order, (1UL << buf_order), 
			sizeof(struct fipc_message));
		return -EINVAL;
	}
	/*
	 * Initialize slots as available
	 */
	for (i = 0; i < nr_slots(buf_order); i++) {
		msg_buffer_1[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer_2[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
	}
	return 0;
}
EXPORT_SYMBOL(fipc_prep_buffers);

static void ring_buf_init(struct fipc_ring_buf *ring_buf,
			unsigned int buf_order,
			void *buffer)
{
	ring_buf->buffer = buffer;
	ring_buf->order_two_mask = order_two_mask(buf_order);
}

int fipc_ring_channel_init(struct fipc_ring_channel *chnl,
			unsigned int buf_order,
			void *buffer_tx, void *buffer_rx)
{
	/*
	 * Checks at compile time
	 */
	BUILD_BUG_ON_NOT_POWER_OF_2(FIPC_CACHE_LINE_SIZE);
	BUILD_BUG_ON(sizeof(struct fipc_ring_buf) != FIPC_CACHE_LINE_SIZE);
	BUILD_BUG_ON(sizeof(struct fipc_message) != FIPC_CACHE_LINE_SIZE);
	/*
	 * Buffers must be as big as one ipc message slot
	 */
	if ((1UL << buf_order) < sizeof(struct fipc_message)) {
		FIPC_DEBUG(FIPC_DEBUG_ERR,
			"Buffers are too small (buf_order = %d, so their size is 2^buf_order = %lu bytes); but one fipc message is %lu, so the buffers need to be at least that big\n",
			buf_order, (1UL << buf_order), 
			sizeof(struct fipc_message));
		return -EINVAL;
	}
	/*
	 * Initialize tx and rx
	 */
	memset(chnl, 0, sizeof(*chnl));
	ring_buf_init(&chnl->tx, buf_order, buffer_tx);
	ring_buf_init(&chnl->rx, buf_order, buffer_rx);

	return 0;
}
EXPORT_SYMBOL(fipc_ring_channel_init);

int fipc_send_msg_start(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	if (check_tx_slot_available(get_current_tx_slot(chnl))) {

		*msg = get_current_tx_slot(chnl);
		inc_tx_slot(chnl);
		ret = 0;

	}

#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	if (!ret)
		fipc_debug("Allocated a slot at index %lu in tx\n",
			tx_msg_to_idx(chnl, *msg));
	else
		fipc_debug("Failed to get a slot, out of slots right now.\n");

#endif

	return ret;
}
EXPORT_SYMBOL(fipc_send_msg_start);

int fipc_send_msg_end(struct fipc_ring_channel *chnl, 
		struct fipc_message *msg)
{
	msg->msg_status = FIPC_MSG_STATUS_SENT;

#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	fipc_debug("Marking message at idx %lu as sent\n",
		tx_msg_to_idx(chnl, msg));

#endif

	return 0;
}
EXPORT_SYMBOL(fipc_send_msg_end);

static int recv_msg_peek(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	if (check_rx_slot_msg_waiting(get_current_rx_slot(chnl))) {

		*msg = get_current_rx_slot(chnl);
		ret = 0;

	}

	return ret;
}

int fipc_recv_msg_start(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret;
	struct fipc_message *m;

	ret = recv_msg_peek(chnl, &m);
	if (!ret) {
		/* Message waiting to be received */
		*msg = m;
		inc_rx_slot(chnl);
	}

#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	if (!ret)
		fipc_debug("Received a message at index %lu in rx\n",
			rx_msg_to_idx(chnl, *msg));
	else
		fipc_debug("No messages to receive right now\n");

#endif

	return ret;
}
EXPORT_SYMBOL(fipc_recv_msg_start);

int fipc_recv_msg_if(struct fipc_ring_channel *chnl,
		int (*pred)(struct fipc_message *, void *),
		void *data,
		struct fipc_message **msg)
{
	int ret;
	struct fipc_message *m;

	ret = recv_msg_peek(chnl, &m);
	if (!ret) {
		/* Message waiting to be received; query predicate */
		if (pred(m, data)) {
			/* Caller wants the message */
			*msg = m;
			inc_rx_slot(chnl);
			ret = 0;
		} else {
			ret = -ENOMSG;
		}
	}

#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	if (!ret)
		fipc_debug("Received a message at index %lu in rx\n",
			rx_msg_to_idx(chnl, *msg));
	else
		fipc_debug("No messages to receive right now, or caller doesn't want it\n");

#endif

	return ret;
}
EXPORT_SYMBOL(fipc_recv_msg_if);

int fipc_recv_msg_end(struct fipc_ring_channel *chnl,
		struct fipc_message *msg)
{
	msg->msg_status = FIPC_MSG_STATUS_AVAILABLE;

#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	fipc_debug("Marking message at idx %lu as received\n",
		rx_msg_to_idx(chnl, msg));

#endif

	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_end);

int fipc_init(void)
{
	FIPC_DEBUG(FIPC_DEBUG_VERB,
		"libfipc initialized\n");

	return 0;
}
EXPORT_SYMBOL(fipc_init);

void fipc_fini(void)
{
	FIPC_DEBUG(FIPC_DEBUG_VERB,
		"libfipc torn down\n");

	return;
}
EXPORT_SYMBOL(fipc_fini);
