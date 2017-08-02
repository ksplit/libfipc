/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void request ( void )
{
	// Write Request
	req_line.regs[0] = req_sequence;

	// Read Response
	while ( likely(resp_line.regs[0] != req_sequence) )
		fipc_test_pause();

	req_sequence++;
}

static inline
void request_send ( void )
{
	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	// Write Request
	req_line.regs[0] = req_sequence;

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);

	// Read Response
	while ( likely(resp_line.regs[0] != req_sequence) )
		fipc_test_pause();

	req_sequence++;
}

static inline
void request_recv ( void )
{
	// Write Request
	req_line.regs[0] = req_sequence;

	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	// Read Response
	while ( likely(resp_line.regs[0] != req_sequence) )
		fipc_test_pause();

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);

	req_sequence++;
}

static inline
void respond ( void )
{
	// Read Request
	while ( likely(req_line.regs[0] != resp_sequence ))
		fipc_test_pause();

	// Write Response
	resp_line.regs[0] = resp_sequence;
	
	resp_sequence++;
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

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		request();

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);

	// Read count
	for ( i = 0; i < ev_num; ++i )
		READ_PMC(&ev_val[i], i);

	// Print count
	pr_err("-------------------------------------------------\n");

	for ( i = 0; i < ev_num; ++i )
		pr_err("Event id:%2x   mask:%2x   count: %llu\n", ev_idx[i], ev_msk[i], ev_val[i]);
	
	pr_err("-------------------------------------------------\n");

	// Reset count
	for ( i = 0; i < ev_num; ++i )
		RESET_COUNTER(i);

	// ================================
	// Send Test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		request_send();

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
		request_recv();

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
		respond();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		respond();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		respond();

	// End test
	fipc_test_thread_release_control_of_CPU();
	complete( &responder_comp );
	return 0;
}


int main ( void )
{
	init_completion( &requester_comp );
	init_completion( &responder_comp );

	// Init Variables
	req_line.regs[0]  = 0;
	resp_line.regs[0] = 0;

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
	requester_thread = fipc_test_thread_spawn_on_CPU ( requester, NULL, requester_cpu );
	if ( requester_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	responder_thread = fipc_test_thread_spawn_on_CPU ( responder, NULL, responder_cpu );
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
