/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}
}

static inline
void request_send ( header_t* chan )
{
	message_t* request;
	message_t* response;
	int i;

	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}
}

static inline
void request_recv ( header_t* chan )
{
	message_t* request;
	message_t* response;
	int i;

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);
}

static inline
void respond ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &request );
		fipc_recv_msg_end( chan, request );
	}

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_send_start( chan, &response );
		fipc_send_msg_end( chan, response );
	}
}

int requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;

	// Program the events to count
	uint i;
	for ( i = 0; i < ev_num; ++i )
		FILL_EVENT_OS(&ev[i], ev_idx[i], ev_msk[i]);

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	start = RDTSC_START();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		request( chan );

	end = RDTSCP();

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);

	// Read count
	for ( i = 0; i < ev_num; ++i )
		READ_PMC(&ev_val[i], i);

	// Print count
	pr_err("-------------------------------------------------\n");

	pr_err("Average Cycles: %llu\n", (end - start) / transactions);

	for ( i = 0; i < ev_num; ++i )
		pr_err("Event id:%2x   mask:%2x   count: %llu\n", ev_idx[i], ev_msk[i], ev_val[i]);
	
	pr_err("-------------------------------------------------\n");

	// Reset count
	for ( i = 0; i < ev_num; ++i )
		RESET_COUNTER(i);

	// ================================
	// Send Test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		request_send( chan );

	// Read count
	for ( i = 0; i < ev_num; ++i )
		READ_PMC(&ev_val[i], i);

	// Print count
	pr_err("-------------------------------------------------\n");

	for ( i = 0; i < ev_num; ++i )
		pr_err("(send) Event id:%2x   mask:%2x   count: %llu\n", ev_idx[i], ev_msk[i], ev_val[i]);
	
	pr_err("-------------------------------------------------\n");

	// Reset count
	for ( i = 0; i < ev_num; ++i )
		RESET_COUNTER(i);

	// ================================
	// Recv Test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		request_recv( chan );

	// Read count
	for ( i = 0; i < ev_num; ++i )
		READ_PMC(&ev_val[i], i);

	// Print count
	pr_err("-------------------------------------------------\n");

	for ( i = 0; i < ev_num; ++i )
		pr_err("(recv) Event id:%2x   mask:%2x   count: %llu\n", ev_idx[i], ev_msk[i], ev_val[i]);
	
	pr_err("-------------------------------------------------\n");

	// Reset count
	for ( i = 0; i < ev_num; ++i )
		RESET_COUNTER(i);

	// End test
	fipc_test_thread_release_control_of_CPU();
	complete( &requester_comp );
	return 0;
}

int responder ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		respond( chan );

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		respond( chan );

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		respond( chan );

	// End test
	fipc_test_thread_release_control_of_CPU();
	complete( &responder_comp );
	return 0;
}


int main ( void )
{
	init_completion( &requester_comp );
	init_completion( &responder_comp );

	header_t*  requester_header = NULL;
	header_t*  responder_header = NULL;
	kthread_t* requester_thread = NULL;
	kthread_t* responder_thread = NULL;

	fipc_init();
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header, &responder_header );

	if ( requester_header == NULL || responder_header == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}

	// Create Threads
	requester_thread = fipc_test_thread_spawn_on_CPU ( requester, requester_header, requester_cpu );
	if ( requester_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	responder_thread = fipc_test_thread_spawn_on_CPU ( responder, responder_header, responder_cpu );
	if ( responder_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// Start threads
	wake_up_process( requester_thread );
	wake_up_process( responder_thread );
	
	// Wait for thread completion
	wait_for_completion( &requester_comp );
	wait_for_completion( &responder_comp );
	
	// Clean up
	fipc_test_thread_free_thread( requester_thread );
	fipc_test_thread_free_thread( responder_thread );
	fipc_test_free_channel( CHANNEL_ORDER, requester_header, responder_header );
	fipc_fini();
	return 0;
}

int init_module(void)
{
    return main();
}

void cleanup_module(void)
{
	return;
}

MODULE_LICENSE("GPL");
