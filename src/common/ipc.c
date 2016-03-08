/*
 * ipc.c
 *
 * Authors: Anton Burtsev, Scotty Bauer
 * Date:    October 2011,  Feburary 2015
 *
 * Copyright: University of Utah
 */

#include <libfipc.h>
#include <libfipc_internal.h>

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
	unsigned long idx = rc->tx.slot & rc->tx.order_two_mask;
	return &rc->tx.buffer[idx];
}

static inline struct fipc_message* 
get_current_rx_slot(struct fipc_ring_channel *rc)
{
	unsigned long idx = rc->rx.slot & rc->rx.order_two_mask;
	return &rc->rx.buffer[idx];
}

static inline int check_rx_slot_available(struct ipc_message *slot)
{
	return (likely(slot->msg_status != FIPC_MSG_STATUS_SENT));
}


static inline int check_tx_slot_available(struct fipc_message *slot)
{
	return unlikely(slot->msg_status != FIPC_MSG_STATUS_AVAILABLE);
}

static inline unsigned long nr_slots(unsigned int buf_order)
{
	return (1UL << buf_order) / sizeof(struct fipc_ring_buf);
}

static inline unsigned long order_two_mask(unsigned int buf_order)
{
	return  nr_slots(buf_order) - 1;
}

int fipc_prep_buffers(unsigned int buf_order, void *buffer_1, void *buffer_2);
{
	unsigned long i;
	struct ipc_message *msg_buffer_1 = buffer_1;
	struct ipc_message *msg_buffer_2 = buffer_2;
	/*
	 * Buffers must be at least as big as one ipc message slot
	 */
	if ((1UL << buf_order) < sizeof(struct fipc_message))
		return -EINVAL;
	/*
	 * Initialize slots as available
	 */
	for (i = 0; i < nr_slots(buf_order); i++) {
		msg_buffer_1[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer_2[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
	}
	return 0;
}

static void ring_buf_init(struct fipc_ring_buf *ring_buf,
			unsigned int buf_order,
			void *buffer)
{
	fipc_mutex_init(&ring_buf->lock);
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
	if ((1UL << buf_order) < sizeof(struct fipc_message))
		return -EINVAL;
	/*
	 * Initialize tx and rx
	 */
	memset(chnl, 0, sizeof(*chnl));
	ring_buf_init(&chnl->tx, buf_order, buffer_tx);
	ring_buf_init(&chnl->rx, buf_order, buffer_rx);

	return 0;
}

int fipc_send_msg_start(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	fipc_mutex_lock(&chnl->tx.lock);

	if (check_tx_slot_available(get_current_tx_slot(chnl))) {

		*msg = get_current_tx_slot(chnl);
		inc_tx_slot(chnl);
		ret = 0;

	}

	fipc_mutex_unlock(&chnl->tx.lock);

	return ret;
}

int fipc_send_msg_end(struct fipc_ring_channel *chnl, 
		struct fipc_message *msg)
{
	msg->msg_status = FIPC_MSG_STATUS_SENT;
	return 0;
}

/* Expects rx to be locked! */
static int recv_msg_peek(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	if (check_rx_slot_available(get_current_rx_slot(chnl))) {

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

	fipc_mutex_lock(&chnl->rx.lock);

	ret = recv_msg_peek(chnl, &m);
	if (!ret) {
		/* Message waiting to be received */
		*msg = m;
		inc_rx_slot(chnl);
	}

	fipc_mutex_unlock(&chnl->rx.lock);

	return ret;
}

int fipc_recv_msg_if(struct fipc_ring_channel *chnl,
		int (*pred)(struct fipc_message *, void *),
		void *data,
		struct fipc_message **msg)
{
	int ret;
	struct fipc_message *m;

	fipc_mutex_lock(&chnl->rx.lock);

	ret = recv_msg_peek(chnl, &m);
	if (!ret) {
		/* Message waiting to be received; query predicate */
		if (pred(m, data)) {
			/* Caller wants the message */
			*msg = m;
			inc_rx_slot(chnl);
		} else {
			ret = -ENOMSG;
		}
	}

	fipc_mutex_unlock(&chnl->rx.lock);

	return ret;
}

int fipc_recv_msg_end(struct fipc_ring_channel *chnl,
		struct fipc_message *msg)
{
	msg->msg_status = FIPC_MSG_STATUS_AVAILABLE;
	return 0;
}

int fipc_init(void)
{
	return 0;
}

void fipc_fini(void)
{
	return;
}
