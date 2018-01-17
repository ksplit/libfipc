/**
 * @File     : ipc.c
 * @Author   : Anton Burtsev		October  2011
 * @Author   : Scotty Bauer			February 2015
 * @Author   : Charles Jacobsen		March    2016
 * @Author   : Abdullah Younis		January  2017
 * @Copyright: University of Utah
 *
 * Asynchronous fast-IPC library using shared memory.
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
#define FIPC_MSG_STATUS_AVAILABLE 0x0UL
#define FIPC_MSG_STATUS_SENT      0x1UL

// =============================================================
// ------------------- HELPER FUNCTIONS ------------------------
// =============================================================

static inline
uint64_t msg_to_idx ( header_t* head, message_t* msg )
{
	return (msg - &head->buffer[0].line[0])/2;
}

static inline
uint64_t nr_slots ( uint32_t buf_order )
{
	return (1UL << buf_order) / sizeof(pair_t);
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
	if ( (1UL << buf_order) < sizeof(pair_t) )
	{
		FIPC_DEBUG(FIPC_DEBUG_ERR,
			"Buffers are too small (buf_order = %u, so their size is 2^buf_order = %llu bytes); but one fipc message is %llu, so the buffers need to be at least that big\n",
			buf_order,
			(1ULL << (unsigned long long)buf_order),
			(unsigned long long)sizeof(message_t) );

		return 1;	// Yes, this is an invalid size.
	}

	return 0;	// No, this is not an invalid size.
}

static inline
message_t* get_current_tx_slot ( header_t* head )
{
	return &head->buffer[head->tx_idx].line[head->tx_side];
}

static inline
message_t* get_current_rx_slot ( header_t* head )
{
	return &head->buffer[head->rx_idx].line[head->tx_side^1];
}

// =============================================================
// --------------------- MAIN FUNCTIONS ------------------------
// =============================================================

int fipc_buffer_init ( uint32_t buf_order, void* buffer )
{
	uint64_t i;
	pair_t* msg_buffer = (pair_t*) buffer;

	//Buffer must be at least as big as one fipc pair slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// Initialize slots as available
	for ( i = 0; i < nr_slots(buf_order); i++ )
	{
		msg_buffer[i].line[0].msg_status = FIPC_MSG_STATUS_AVAILABLE;
		msg_buffer[i].line[1].msg_status = FIPC_MSG_STATUS_AVAILABLE;
	}

	return 0;
}
EXPORT_SYMBOL(fipc_prep_buffer);

int fipc_channel_init ( header_t* h1, header_t* h2, uint32_t buf_order, void* buffer )
{
	// Compile-time Assertions
	FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(message_t) != FIPC_CACHE_LINE_SIZE);

	//Buffer must be at least as big as one fipc pair slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// Header initialization
	memset( h1, 0, sizeof( header_t ) );
	memset( h2, 0, sizeof( header_t ) );

	h1->tx_side = 0;
	h1->mask    = order_two_mask( buf_order );
	h1->buffer  = (pair_t*) buffer;

	h2->tx_side = 1;
	h2->mask    = order_two_mask( buf_order );
	h2->buffer  = (pair_t*) buffer;

	return 0;
}
EXPORT_SYMBOL(fipc_ring_channel_init);

int fipc_header_init ( header_t* head, int client, uint32_t buf_order, void* buffer )
{
	// Compile-time Assertions
	FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(FIPC_CACHE_LINE_SIZE);
	FIPC_BUILD_BUG_ON(sizeof(message_t) != FIPC_CACHE_LINE_SIZE);

	//Buffer must be at least as big as one fipc pair slot
	if ( invalid_buf_order_size( buf_order ) )
		return -EINVAL;

	// Header initialization
	memset( head, 0, sizeof( header_t ) );

	head->tx_side = (client == 0 ? 0 : 1);
	head->mask    = order_two_mask( buf_order );
	head->buffer  = (pair_t*) buffer;

	return 0;
}
EXPORT_SYMBOL(fipc_ring_channel_init);

int fipc_send_msg_start ( header_t* head, message_t** msg )
{
	if ( get_current_tx_slot( head )->msg_status != FIPC_MSG_STATUS_AVAILABLE )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Failed to get a slot, out of slots right now.\n");
		return -EWOULDBLOCK;
	}

	*msg = get_current_tx_slot( head );
	head->tx_idx++;
	FIPC_DEBUG(FIPC_DEBUG_VERB, "Allocated a slot at index %llu in tx\n", (unsigned long long) msg_to_idx( head, *msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_send_msg_start);

int fipc_send_msg_end ( header_t* head, message_t* msg )
{
	msg->msg_status = FIPC_MSG_STATUS_SENT;
	FIPC_DEBUG(FIPC_DEBUG_VERB, "Marking message at idx %llu as sent\n", (unsigned long long) msg_to_idx( head, msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_send_msg_end);

int fipc_recv_msg_start ( header_t* head, message_t** msg )
{
	if ( get_current_rx_slot( head )->msg_status != FIPC_MSG_STATUS_SENT )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "No messages to received right now\n");
		return -EWOULDBLOCK;
	}

	*msg = get_current_rx_slot( head );
	head->rx_idx++;

	FIPC_DEBUG(FIPC_DEBUG_VERB, "Received a message at index %llu in rx\n", (unsigned long long) msg_to_idx( head, *msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_start);

int fipc_recv_msg_if ( header_t* head, int (*pred)(message_t*, void*), void* data, message_t** msg )
{
	if ( get_current_rx_slot( head )->msg_status != FIPC_MSG_STATUS_SENT )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "No messages to received right now\n");
		return -EWOULDBLOCK;
	}

	if ( ! pred( get_current_rx_slot( head ), data ) )
	{
		FIPC_DEBUG(FIPC_DEBUG_VERB, "Message failed predicate; not received\n");
		return -ENOMSG;
	}

	*msg = get_current_rx_slot( head );
	head->rx_idx++;

	FIPC_DEBUG(FIPC_DEBUG_VERB, "Received a message at index %llu in rx\n", (unsigned long long) msg_to_idx( head, *msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_if);

int fipc_recv_msg_end ( header_t* head, message_t* msg )
{
	msg->msg_status = FIPC_MSG_STATUS_AVAILABLE;
	FIPC_DEBUG(FIPC_DEBUG_VERB, "Marking message at idx %llu as received\n", (unsigned long long) msg_to_idx( head, msg ));
	return 0;
}
EXPORT_SYMBOL(fipc_recv_msg_end);