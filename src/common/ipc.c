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
#include <linux/spinlock.h>
#include <asm/mwait.h>

#ifdef LCD_DOMAINS
#include <lcd_config/post_hook.h>
#endif

#ifndef LINUX_KERNEL_MODULE
#undef EXPORT_SYMBOL
#define EXPORT_SYMBOL(x)
#endif

static inline unsigned long get_tx_slot(struct fipc_ring_channel *rc)
{
	return rc->tx.slot;
}

#define FIPC_MSG_STATUS_AVAILABLE 0xdeaddeadUL
#define FIPC_MSG_STATUS_SENT      0xfeedfeedUL

static inline unsigned long get_tx_idx(struct fipc_ring_channel *rc)
{
	return rc->tx.slot & rc->tx.order_two_mask;
}

static inline unsigned long get_rx_idx(struct fipc_ring_channel *rc)
{
	return rc->rx.slot & rc->rx.order_two_mask;
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

#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

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

#endif

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

int 
LIBFIPC_FUNC_ATTR
fipc_prep_buffers(unsigned int buf_order, void *buffer_1, void *buffer_2)
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

int 
LIBFIPC_FUNC_ATTR
fipc_ring_channel_init(struct fipc_ring_channel *chnl,
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

inline int
LIBFIPC_FUNC_ATTR
fipc_send_msg_start(struct fipc_ring_channel *chnl,
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

inline int
LIBFIPC_FUNC_ATTR
fipc_send_msg_end(struct fipc_ring_channel *chnl, 
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

static inline int recv_msg_peek(struct fipc_ring_channel *chnl,
			struct fipc_message **msg)
{
	int ret = -EWOULDBLOCK;

	if (check_rx_slot_msg_waiting(get_current_rx_slot(chnl))) {

		*msg = get_current_rx_slot(chnl);
		ret = 0;

	}

	return ret;
}

inline int
LIBFIPC_FUNC_ATTR
fipc_recv_msg_mwait(struct fipc_ring_channel *chnl,
		struct fipc_message **msg)
{
	int ret;

	while(1) {

		if(check_rx_slot_msg_waiting(get_current_rx_slot(chnl))) {
			goto out;
		
		} else {
			__monitor((void *)&get_current_rx_slot(chnl)->msg_status, 0, 0);
			//printk("armed monitor.\n");

			//if(check_rx_slot_msg_waiting(get_current_rx_slot(chnl))) {
			//	goto out;
			//}
			//printk("mwaiting..\n");
			__mwait(0, 0);
			//printk("outof mwait\n");
		}
	
	}
out:
	*msg = get_current_rx_slot(chnl);
	inc_rx_slot(chnl);
	ret = 0;


#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	if (!ret)
		fipc_debug("Received a message at index %lu in rx\n",
			rx_msg_to_idx(chnl, *msg));
	else
		fipc_debug("No messages to receive right now\n");

#endif


	return ret;
}
EXPORT_SYMBOL(fipc_recv_msg_mwait);

int 
LIBFIPC_FUNC_ATTR
fipc_recv_msg_start(struct fipc_ring_channel *chnl,
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

typedef enum {
    msg_type_unspecified,
    msg_type_request,
    msg_type_response,
} msg_type_t;


/* Message type is in low 2 bits of flags */
static inline uint32_t thc_get_msg_type(struct fipc_message *msg)
{
	return fipc_get_flags(msg) & 0x3;
}

#define AWE_TABLE_ORDER 10
static inline uint32_t thc_get_msg_id(struct fipc_message *msg)
{
	/* shift off type bits, and mask off msg id */
	return (fipc_get_flags(msg) >> 2) & ((1 << AWE_TABLE_ORDER) - 1);
}

inline
int fipc_nonblocking_recv_start_if(struct fipc_ring_channel *channel,
				struct fipc_message** out)
{
retry:
	while ( 1 )
	{
		// Poll until we get a message or error
		*out = get_current_rx_slot( channel);

		if (!check_rx_slot_msg_waiting(*out)) {
			// message not for us
			return 2;
		}
		break;
	}

	if( likely(thc_get_msg_type(*out) == (uint32_t)msg_type_request ))
	{
		inc_rx_slot (channel);
		return 0;
	}
	else
	{
#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB
		fipc_debug("%s:%d msg not for us!\n", __func__, __LINE__);
#endif
		return 1; //message not for this awe
	}
	goto retry;
	return 0;
}
EXPORT_SYMBOL(fipc_nonblocking_recv_start_if);

int
LIBFIPC_FUNC_ATTR
fipc_recv_msg_poll(struct fipc_ring_channel *chnl,
		struct fipc_message **msg, uint32_t *received_cookie)
{
	int ret;
	struct fipc_message *m;

	ret = recv_msg_peek(chnl, &m);
	if (!ret) {
		/* Message waiting to be received; query predicate */
		if (thc_get_msg_type(m) == (uint32_t) msg_type_request) {
			/* Caller wants the message */
			*msg = m;
			inc_rx_slot(chnl);
			ret = 0;
		} else {
			*received_cookie = thc_get_msg_id(m);
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
EXPORT_SYMBOL(fipc_recv_msg_poll);

int 
LIBFIPC_FUNC_ATTR
fipc_recv_msg_klcd_if(struct fipc_ring_channel *chnl,
		int (*pred)(struct fipc_message *, void *),
		void *data,
		struct fipc_message **msg)
{
	int ret;
	struct fipc_message *m;
	//unsigned long flags = 0;

	//spin_lock_irqsave(&lock, flags);
	//spin_lock(&lock);
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

	//spin_unlock_irqrestore(&lock, flags);
	//spin_unlock(&lock);
#if FIPC_DEBUG_LVL >= FIPC_DEBUG_VERB

	if (!ret)
		fipc_debug("Received a message at index %lu in rx\n",
			rx_msg_to_idx(chnl, *msg));
	else
		fipc_debug("No messages to receive right now, or caller doesn't want it\n");

#endif

	return ret;
}
EXPORT_SYMBOL(fipc_recv_msg_klcd_if);


int 
LIBFIPC_FUNC_ATTR
fipc_recv_msg_if(struct fipc_ring_channel *chnl,
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

inline int
LIBFIPC_FUNC_ATTR
fipc_recv_msg_end(struct fipc_ring_channel *chnl,
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

int 
LIBFIPC_FUNC_ATTR
fipc_init(void)
{
	FIPC_DEBUG(FIPC_DEBUG_VERB,
		"libfipc initialized\n");

	return 0;
}
EXPORT_SYMBOL(fipc_init);

void 
LIBFIPC_FUNC_ATTR
fipc_fini(void)
{
	FIPC_DEBUG(FIPC_DEBUG_VERB,
		"libfipc torn down\n");

	return;
}
EXPORT_SYMBOL(fipc_fini);
