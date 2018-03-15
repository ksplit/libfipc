/*
 * ipc_one-slot.c
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

static inline struct fipc_message* 
get_current_tx_slot_0(struct fipc_ring_channel *rc)
{
	return &rc->tx.buffer[0];
}

static inline struct fipc_message* 
get_current_rx_slot_0(struct fipc_ring_channel *rc)
{
	return &rc->rx.buffer[0];
}

static inline unsigned long nr_slots_0(unsigned int buf_size)
{
	return (buf_size) / sizeof(struct fipc_ring_buf);
}

static inline int check_rx_slot_msg_waiting(struct fipc_message *slot)
{
	return slot->msg_status == FIPC_MSG_STATUS_SENT;
}

static inline int check_tx_slot_available(struct fipc_message *slot)
{
	return slot->msg_status == FIPC_MSG_STATUS_AVAILABLE;
}

int 
LIBFIPC_FUNC_ATTR
fipc_prep_buffers_0(unsigned int buf_size, void *buffer_1, void *buffer_2)
{
	unsigned long i;
	struct fipc_message *msg_buffer_1 = buffer_1;
	struct fipc_message *msg_buffer_2 = buffer_2;
	/*
	 * Buffers must be at least as big as one ipc message slot
	 */
	if (buf_size < sizeof(struct fipc_message)) {
		FIPC_DEBUG(FIPC_DEBUG_ERR,
			"Buffers are too small (buf_size = %d; but one fipc message is %lu, so the buffers need to be at least that big\n",
			buf_size, sizeof(struct fipc_message));
		return -EINVAL;
	}
	/*
	 * Initialize slots as available
	 */
	for (i = 0; i < nr_slots_0(buf_size); i++) {
		msg_buffer_1[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer_2[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
	}
	return 0;
}
EXPORT_SYMBOL(fipc_prep_buffers_0);

static void ring_buf_init_0(struct fipc_ring_buf *ring_buf,
			unsigned int buf_size,
			void *buffer)
{
	ring_buf->buffer = buffer;
//	ring_buf->order_two_mask = order_two_mask(buf_order);
}

int 
LIBFIPC_FUNC_ATTR
fipc_ring_channel_init_0(struct fipc_ring_channel *chnl,
		unsigned int buf_size,
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
	if (buf_size < sizeof(struct fipc_message)) {
		FIPC_DEBUG(FIPC_DEBUG_ERR,
			"Buffers are too small (buf_size = %d bytes); but one fipc message is %lu, so the buffers need to be at least that big\n",
			buf_size, sizeof(struct fipc_message));
		return -EINVAL;
	}
	/*
	 * Initialize tx and rx
	 */
	memset(chnl, 0, sizeof(*chnl));
	ring_buf_init_0(&chnl->tx, buf_size, buffer_tx);
	ring_buf_init_0(&chnl->rx, buf_size, buffer_rx);

	return 0;
}
EXPORT_SYMBOL(fipc_ring_channel_init_0);

static int recv_msg_peek_0(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	if (check_rx_slot_msg_waiting(get_current_rx_slot_0(chnl))) {

		*msg = get_current_rx_slot_0(chnl);
		ret = 0;

	}

	return ret;
}

int 
LIBFIPC_FUNC_ATTR
fipc_recv_msg_start_0(struct fipc_ring_channel *chnl,
		struct fipc_message **msg)
{
	int ret;
	struct fipc_message *m;

	ret = recv_msg_peek_0(chnl, &m);
	if (!ret) {
		/* Message waiting to be received */
		*msg = m;
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
EXPORT_SYMBOL(fipc_recv_msg_start_0);

int 
LIBFIPC_FUNC_ATTR
fipc_send_msg_start_0(struct fipc_ring_channel *chnl,
		struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	if (check_tx_slot_available(get_current_tx_slot_0(chnl))) {

		*msg = get_current_tx_slot_0(chnl);
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
EXPORT_SYMBOL(fipc_send_msg_start_0);

int 
LIBFIPC_FUNC_ATTR
fipc_recv_msg_if_0(struct fipc_ring_channel *chnl,
		int (*pred)(struct fipc_message *, void *),
		void *data,
		struct fipc_message **msg)
{
	int ret;
	struct fipc_message *m;

	ret = recv_msg_peek_0(chnl, &m);
	if (!ret) {
		/* Message waiting to be received; query predicate */
		if (pred(m, data)) {
			/* Caller wants the message */
			*msg = m;
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
EXPORT_SYMBOL(fipc_recv_msg_if_0);

