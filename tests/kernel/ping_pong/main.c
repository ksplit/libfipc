/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	fipc_test_blocking_send_start( chan, &request );
	fipc_send_msg_end ( chan, request );

	fipc_test_blocking_recv_start( chan, &response );
	fipc_recv_msg_end( chan, response );
}

void respond ( header_t* chan )
{
	message_t* request;
	message_t* response;

	fipc_test_blocking_recv_start( chan, &request );
	fipc_recv_msg_end( chan, request );

	fipc_test_blocking_send_start( chan, &response );
	fipc_send_msg_end( chan, response );
}

int requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t  CACHE_ALIGNED transaction_id;
	register uint64_t  CACHE_ALIGNED start;
	register uint64_t  CACHE_ALIGNED end;
	register int32_t*  CACHE_ALIGNED times = kmalloc( TRANSACTIONS * sizeof( int32_t ), GFP_KERNEL );

	// Begin test
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		request( chan );

		end = RDTSCP();
		times[transaction_id] = (end - start);
	}

	// End test
	fipc_test_stat_get_and_print_stats( times, TRANSACTIONS );
	kfree( times );
	complete( &requester_comp );
	return 0;
}

int responder ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond( chan );
	}

	// End test
	complete( &responder_comp );
	return 0;
}


int main ( void )
{
	header_t* requester_header = NULL;
	header_t* responder_header = NULL;
	struct task_struct* requester_thread = NULL;
	struct task_struct* responder_thread = NULL;

	fipc_init();
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header, &responder_header );

	if ( requester_header == NULL || responder_header == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}

	// Create Threads
	requester_thread = fipc_test_thread_spawn_on_CPU ( requester, requester_header, REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	responder_thread = fipc_test_thread_spawn_on_CPU ( responder, responder_header, RESPONDER_CPU );
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
	// fipc_test_thread_wait_for_thread( requester_thread );
	// fipc_test_thread_wait_for_thread( responder_thread );
	
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
