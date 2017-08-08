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
	line.regs[0] = MSG_AVAIL;

	// Read Response
	while ( unlikely( line.regs[0] != MSG_READY ) )
		fipc_test_pause();
}

static inline
void request_send ( void )
{
	int i;

	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	// Write Request
	line.regs[0] = MSG_AVAIL;

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);

	// Read Response
	while ( unlikely( line.regs[0] != MSG_READY ) )
		fipc_test_pause();
}

static inline
void request_recv ( void )
{
	int i;

	// Write Request
	line.regs[0] = MSG_AVAIL;

	// Start counting
	for ( i = 0; i < ev_num; ++i )
		PROG_EVENT(&ev[i], i);

	// Read Response
	while ( unlikely( line.regs[0] != MSG_READY ) )
		fipc_test_pause();

	// Stop counting
	for ( i = 0; i < ev_num; ++i )
		STOP_EVENT(i);
}

static inline
void respond ( void )
{
	// Read Request
	while ( unlikely( line.regs[0] != MSG_AVAIL ) )
		fipc_test_pause();

	// Write Response
	line.regs[0] = MSG_READY;
}

int requester ( void* data )
{
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
		request();

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

	kthread_t* requester_thread = NULL;
	kthread_t* responder_thread = NULL;

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
