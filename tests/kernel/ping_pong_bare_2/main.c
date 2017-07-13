/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void request ( void )
{
	// Wait until message is available (GetS)
	// Send request (GetM)
	req_line = req_sequence;

	// Wait until message is received (GetS)
	while ( likely(resp_line != req_sequence) )
		fipc_test_pause();

	req_sequence ++;
}

static inline
void respond ( void )
{
	// Wait until message is received (GetS)
	while ( likely(req_line != resp_sequence ))
		fipc_test_pause();

	// Send response (GetM)
	resp_line = resp_sequence;
	resp_sequence ++;
}

int requester ( void* data )
{
	register uint64_t  CACHE_ALIGNED transaction_id;
	register uint64_t  CACHE_ALIGNED start;
	register uint64_t  CACHE_ALIGNED end;
	register int32_t*  CACHE_ALIGNED times = kmalloc( TRANSACTIONS * sizeof( int32_t ), GFP_KERNEL );

	// Begin test
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		request();

		end = RDTSCP();
		times[transaction_id] = end - start;
	}

	// End test
	fipc_test_stat_get_and_print_stats( times, TRANSACTIONS );
	kfree( times );
	complete( &requester_comp );
	return 0;
}

int responder ( void* data )
{
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond();
	}

	// End test
	complete( &responder_comp );
	return 0;
}


int main ( void )
{
	kthread_t* requester_thread = NULL;
	kthread_t* responder_thread = NULL;

	// Create Threads
	requester_thread = fipc_test_thread_spawn_on_CPU ( requester, NULL, REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	responder_thread = fipc_test_thread_spawn_on_CPU ( responder, NULL, RESPONDER_CPU );
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
