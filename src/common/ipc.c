/**
 * @File     : ipc.c
 * @Author   : Anton Burtsev		October  2011
 * @Author   : Scotty Bauer			February 2015
 * @Author   : Charles Jacobsen		March    2016
 * @Author   : Abdullah Younis		January  2017
 * @Copyright: University of Utah
 *
 * This code implements the main and helper functions of the fipc library.
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
	#undef  EXPORT_SYMBOL
	#define EXPORT_SYMBOL(x)
#endif

// Message Statuses
#define FIPC_MSG_STATUS_AVAILABLE 0xdeadU
#define FIPC_MSG_STATUS_DUMMY     0xfadeU
#define FIPC_MSG_STATUS_SENT      0xfeedU

// =============================================================
// ------------------- HELPER FUNCTIONS ------------------------
// =============================================================

static inline
uint64_t get_tx_slot ( header_t* rc )
{
	return rc->tx.slot;
}

static inline
uint64_t get_tx_idx ( header_t* rc )
{
	return rc->tx.slot & rc->tx.mask;
}

static inline
uint64_t get_rx_idx ( header_t* rc )
{
	return rc->rx.slot & rc->rx.mask;
}

static inline
uint64_t inc_tx_slot ( header_t* rc )
{
	return (rc->tx.slot++);
}

static inline
uint64_t inc_rx_slot ( header_t* rc )
{
	return (rc->rx.slot++);
}

static inline
void add_tx_slot ( header_t* rc, int amount )
{
	rc->tx.slot += amount;
}

static inline
void add_rx_slot ( header_t* rc, int amount )
{
	rc->rx.slot += amount;
}

static inline
message_t* get_tx_slot ( header_t* rc, uint64_t index )
{
	return &rc->tx.buffer[index];
}

static inline
message_t* get_current_tx_slot ( header_t* rc )
{
	return &rc->tx.buffer[get_tx_idx(rc)];
}

static inline
message_t* get_current_rx_slot ( header_t* rc )
{
	return &rc->rx.buffer[get_rx_idx(rc)];
}

static inline
uint64_t rx_msg_to_idx ( header_t *rc, message_t *msg )
{
	return msg - rc->rx.buffer;
}

static inline
uint64_t tx_msg_to_idx ( header_t *rc, message_t *msg )
{
	return msg - rc->tx.buffer;
}

static inline
int check_rx_slot_msg_waiting ( message_t *slot )
{
	return slot->msg_status == FIPC_MSG_STATUS_SENT;
}

static inline
int check_rx_slot_msg_dummy ( message_t *slot )
{
	return slot->msg_status == FIPC_MSG_STATUS_DUMMY;
}

static inline
int check_tx_slot_available ( message_t *slot )
{
	return slot->msg_status == FIPC_MSG_STATUS_AVAILABLE;
}

static inline
int check_tx_slots_available ( message_t *slot )
{
	return slot->msg_status == FIPC_MSG_STATUS_AVAILABLE;
}

static inline
uint64_t nr_slots ( uint32_t buf_order )
{
	return (1UL << buf_order) / sizeof(buffer_t);
}

static inline
uint64_t order_two_mask ( uint32_t buf_order )
{
	return  nr_slots( buf_order ) - 1;
}

static inline
int invalid_buf_order_size ( uint32_t buf_order )
{
	// Buffers must be at least as big as one ipc message slot
	if ( (1UL << buf_order) < sizeof(message_t) )
	{
		FIPC_DEBUG(FIPC_DEBUG_ERR,
			"Buffers are too small (buf_order = %d, so their size is 2^buf_order = %lu bytes); but one fipc message is %lu, so the buffers need to be at least that big\n",
			buf_order,
			(1UL << buf_order),
			sizeof(message_t) );

		return 1;	// Yes, this is an invalid size.
	}

	return 0;	// No, this is not an invalid size.
}

static inline
void ring_buf_init ( buffer_t* ring_buf, uint32_t buf_order, void* buffer )
{
	ring_buf->buffer = buffer;
	ring_buf->mask   = order_two_mask( buf_order );
}

// =============================================================
// --------------------- MAIN FUNCTIONS ------------------------
// =============================================================

int
LIBFIPC_FUNC_ATTR
fipc_prep_buffer ( uint32_t buf_order, void* buffer )
{
	uint64_t i;
	message_t* msg_buffer = buffer;

	//Buffer must be at least as big as one ipc message slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// Initialize slots as available
	for ( i = 0; i < nr_slots(buf_order); i++ )
	{
		msg_buffer[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer[i].msg_length = 1;
	}

	return 0;
}
EXPORT_SYMBOL(fipc_prep_buffer);

int
LIBFIPC_FUNC_ATTR
fipc_prep_buffers ( uint32_t buf_order, void *buffer_1, void *buffer_2 )
{
	uint64_t i;
	message_t* msg_buffer_1 = buffer_1;
	message_t* msg_buffer_2 = buffer_2;

	//Buffers must be at least as big as one ipc message slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// Initialize slots as available
	for ( i = 0; i < nr_slots(buf_order); i++ )
	{
		msg_buffer_1[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer_1[i].msg_length = 1;

		msg_buffer_2[i].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer_2[i].msg_length = 1;
	}

	return 0;
}
EXPORT_SYMBOL(fipc_prep_buffers);


int
LIBFIPC_FUNC_ATTR
fipc_ring_channel_init ( header_t* chnl, uint32_t buf_order,
							void* buffer_tx, void* buffer_rx )
{
	// Compile-time Assertions
	FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(buffer_t) != FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(message_t) != FIPC_CACHE_LINE_SIZE);

	//Buffers must be at least as big as one ipc message slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// TX and RX initialization
	memset( chnl, 0, sizeof( header_t ) );
	ring_buf_init( &chnl->tx, buf_order, buffer_tx );
	ring_buf_init( &chnl->rx, buf_order, buffer_rx );

	return 0;
}
EXPORT_SYMBOL(fipc_ring_channel_init);

int
LIBFIPC_FUNC_ATTR
fipc_tx_channel_init ( header_t* chnl, uint32_t buf_order, void* buffer_tx )
{
	// Compile-time Assertions
	FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(buffer_t) != FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(message_t) != FIPC_CACHE_LINE_SIZE);

	//Buffers must be at least as big as one ipc message slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// TX and RX initialization
	memset( chnl, 0, sizeof( buffer_t ) );
	ring_buf_init( &chnl->tx, buf_order, buffer_tx );

	return 0;
}
EXPORT_SYMBOL(fipc_tx_channel_init);

int
LIBFIPC_FUNC_ATTR
fipc_rx_channel_init ( header_t* chnl, uint32_t buf_order, void* buffer_rx )
{
	// Compile-time Assertions
	FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(buffer_t) != FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(message_t) != FIPC_CACHE_LINE_SIZE);

	//Buffers must be at least as big as one ipc message slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// TX and RX initialization
	memset( (buffer_t*)chnl + 1, 0, sizeof( buffer_t ) );
	ring_buf_init( &chnl->rx, buf_order, buffer_rx );

	return 0;
}
EXPORT_SYMBOL(fipc_tx_channel_init);

int
LIBFIPC_FUNC_ATTR
fipc_send_msg_start ( header_t *chnl, message_t **msg )
{
	if ( ! check_tx_slot_available ( get_current_tx_slot( chnl ) ) )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Failed to get a slot, out of slots right now.\n");
		return -EWOULDBLOCK;
	}

	*msg = get_current_tx_slot( chnl );
	*msg->msg_length = 1;
	inc_tx_slot( chnl );
	FIPC_DEBUG(FIPC_DEBUG_VERB, "Allocated a slot at index %lu in tx\n", tx_msg_to_idx( chnl, *msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_send_msg_start);

int
LIBFIPC_FUNC_ATTR
fipc_send_long_msg_start ( header_t *chnl, message_t **msg, uint16_t len )
{
	if ( chnl == NULL || chnl->tx == NULL )
		return -EINVAL;

	uint64_t chan_size = chnl->tx.mask + 1;
	uint64_t chan_slot = get_tx_idx( chnl );

	if ( len > chan_size )
		return -EINVAL;

	uint64_t len_iter;
	for ( len_iter = 0; len_iter < len && len_iter < chan_size; ++len_iter )
	{
		if ( ! check_tx_slot_available ( get_tx_slot( chnl, chan_slot + len_iter  ) ) )
		{
			break;
		}
	}

	if ( len_iter >= len )
	{
		*msg = get_current_tx_slot( chnl );
		*msg->msg_length = len;
		add_tx_slot ( chnl, len );
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Allocated %lu slots beginning at index %lu in tx\n", len, chan_slot);
		return 0;
	}

	for ( len_iter = 0; len_iter < len; ++len_iter )
	{
		if ( ! check_tx_slot_available ( get_tx_slot( chnl, len_iter  ) ) )
		{
			FIPC_DEBUG(FIPC_DEBUG_VERB, "Failed to get %lu slots, out of slots right now.\n", len);
			return -EWOULDBLOCK;
		}
	}

	get_current_tx_slot( chnl )->msg_status = FIPC_MSG_STATUS_DUMMY
	get_current_tx_slot( chnl )->msg_length = chan_size - chan_slot;
	*msg = get_tx_slot( chnl, 0 );
	add_tx_slot ( chnl, chan_size - chan_slot + len );
	FIPC_DEBUG(FIPC_DEBUG_VERB, "Allocated %lu slots beginning at index %lu in tx\n", len, 0);
	return 0;
}
EXPORT_SYMBOL(fipc_send_msg_start);

int
LIBFIPC_FUNC_ATTR
fipc_send_msg_end ( header_t *chnl, message_t *msg )
{
	msg->msg_status = FIPC_MSG_STATUS_SENT;
	FIPC_DEBUG(FIPC_DEBUG_VERB, "Marking message at idx %lu as sent\n", tx_msg_to_idx( chnl, msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_send_msg_end);

int
LIBFIPC_FUNC_ATTR
fipc_recv_msg_start ( header_t *chnl, message_t **msg )
{
	message_t* chnl_slot = get_current_rx_slot( chnl );

	if ( check_rx_slot_msg_dummy( chnl_slot ) )
	{
		add_rx_slot( chnl, chnl_slot->msg_length );
		chnl_slot->msg_status = FIPC_MSG_STATUS_AVAILABLE;
		chnl_slot = get_current_rx_slot( chnl );
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Skipped a dummy message.\n");
	}

	if ( ! check_rx_slot_msg_waiting( chnl_slot ) )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "No messages to receive right now\n");
		return -EWOULDBLOCK;
	}

	*msg = chnl_slot
	add_rx_slot( chnl, chnl_slot->msg_length );

	FIPC_DEBUG(FIPC_DEBUG_VERB, "Received a message with length %lu at index %lu in rx\n",
				chnl_slot->msg_length, rx_msg_to_idx( chnl, *msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_start);

int
LIBFIPC_FUNC_ATTR
fipc_recv_msg_if( header_t *chnl, int (*pred)(message_t *, void *),
					void *data, message_t **msg )
{
	message_t* chnl_slot = get_current_rx_slot( chnl );

	if ( check_rx_slot_msg_dummy( chnl_slot ) )
	{
		add_rx_slot( chnl, chnl_slot->msg_length );
		chnl_slot->msg_status = FIPC_MSG_STATUS_AVAILABLE;
		chnl_slot = get_current_rx_slot( chnl );
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Skipped a dummy message.\n");
	}

	if ( ! check_rx_slot_msg_waiting( chnl_slot ) )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "No messages to receive right now\n");
		return -EWOULDBLOCK;
	}

	if ( ! pred( chnl_slot, data ) )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Message failed predicate; not received\n");
		return -ENOMSG;
	}

	*msg = chnl_slot
	add_rx_slot( chnl, chnl_slot->msg_length );

	FIPC_DEBUG(FIPC_DEBUG_VERB, "Received a message with length %lu at index %lu in rx\n",
				chnl_slot->msg_length, rx_msg_to_idx( chnl, *msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_if);

int
LIBFIPC_FUNC_ATTR
fipc_recv_msg_end ( header_t *chnl, message_t *msg )
{
	uint16_t len;
	for ( len = msg->msg_length - 1; len >= 0; --len, ++msg )
	{
		msg->msg_length = 1;
		msg->msg_status = FIPC_MSG_STATUS_AVAILABLE;
	}

	FIPC_DEBUG(FIPC_DEBUG_VERB, "Marking message at idx %lu as received\n", rx_msg_to_idx( chnl, msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_end);

int
LIBFIPC_FUNC_ATTR
fipc_init ( void )
{
	FIPC_DEBUG(FIPC_DEBUG_VERB, "libfipc initialized\n");
	return 0;
}
EXPORT_SYMBOL(fipc_init);

void
LIBFIPC_FUNC_ATTR
fipc_fini ( void )
{
	FIPC_DEBUG(FIPC_DEBUG_VERB, "libfipc torn down\n");
	return;
}
EXPORT_SYMBOL(fipc_fini);
