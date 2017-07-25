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
	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}
}

static inline
void respond ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_recv_start( chan, &request );
		fipc_recv_msg_end( chan, request );
	}

	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_send_start( chan, &response );
		fipc_send_msg_end( chan, response );
	}
}

int requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;

	FILL_EVENT_OS(&e1, 0x26, 0x01);
	FILL_EVENT_OS(&e2, 0x26, 0x02);
	FILL_EVENT_OS(&e3, 0x26, 0x04);
	FILL_EVENT_OS(&e4, 0x26, 0x08);
	FILL_EVENT_OS(&e5, 0x27, 0x01);
	FILL_EVENT_OS(&e6, 0x27, 0x02);
	FILL_EVENT_OS(&e7, 0x27, 0x04);
	FILL_EVENT_OS(&e8, 0x27, 0x08);

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	PROG_EVENT(&e1, EVENT_SEL0);
	PROG_EVENT(&e2, EVENT_SEL1);
	PROG_EVENT(&e3, EVENT_SEL2);
	PROG_EVENT(&e4, EVENT_SEL3);
	PROG_EVENT(&e5, EVENT_SEL4);
	PROG_EVENT(&e6, EVENT_SEL5);
	PROG_EVENT(&e7, EVENT_SEL6);
	PROG_EVENT(&e8, EVENT_SEL7);

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		request( chan );

	STOP_EVENT(EVENT_SEL7);
	STOP_EVENT(EVENT_SEL6);
	STOP_EVENT(EVENT_SEL5);
	STOP_EVENT(EVENT_SEL4);
	STOP_EVENT(EVENT_SEL3);
	STOP_EVENT(EVENT_SEL2);
	STOP_EVENT(EVENT_SEL1);
	STOP_EVENT(EVENT_SEL0);

	READ_PMC(&VAL(e1), EVENT_SEL0);
	READ_PMC(&VAL(e2), EVENT_SEL1);
	READ_PMC(&VAL(e3), EVENT_SEL2);
	READ_PMC(&VAL(e4), EVENT_SEL3);
	READ_PMC(&VAL(e5), EVENT_SEL4);
	READ_PMC(&VAL(e6), EVENT_SEL5);
	READ_PMC(&VAL(e7), EVENT_SEL6);
	READ_PMC(&VAL(e8), EVENT_SEL7);

	pr_err("Event #1: %llu\n", VAL(e1));
	pr_err("Event #2: %llu\n", VAL(e2));
	pr_err("Event #3: %llu\n", VAL(e3));
	pr_err("Event #4: %llu\n", VAL(e4));
	pr_err("Event #5: %llu\n", VAL(e5));
	pr_err("Event #6: %llu\n", VAL(e6));
	pr_err("Event #7: %llu\n", VAL(e7));
	pr_err("Event #8: %llu\n", VAL(e8));

	RESET_COUNTER(EVENT_SEL0);
	RESET_COUNTER(EVENT_SEL1);
	RESET_COUNTER(EVENT_SEL2);
	RESET_COUNTER(EVENT_SEL3);
	RESET_COUNTER(EVENT_SEL4);
	RESET_COUNTER(EVENT_SEL5);
	RESET_COUNTER(EVENT_SEL6);
	RESET_COUNTER(EVENT_SEL7);


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

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond( chan );
	}

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
