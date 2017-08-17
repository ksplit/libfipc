/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void request ( uint64_t index )
{
	// Write Request
	cache[index].regs[0] = MSG_AVAIL;

	// Read Response
	while ( unlikely( cache[index].regs[0] != MSG_READY ) )
		fipc_test_pause();
}

static inline
void respond ( uint64_t index )
{
	// Read Request
	while ( unlikely( cache[index].regs[0] != MSG_AVAIL ) )
		fipc_test_pause();

	// Write Response
	cache[index].regs[0] = MSG_READY;
}

int requester ( void* data )
{
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t CACHE_ALIGNED correction = fipc_test_time_get_correction();
	register int32_t* CACHE_ALIGNED times = vmalloc( transactions * sizeof( int32_t ) );

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		start = RDTSC_START();

		request( transaction_id );

		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
	}

	// End test
	fipc_test_thread_release_control_of_CPU();
	fipc_test_stat_get_and_print_stats( times, transactions );
	vfree( times );
	complete( &requester_comp );
	return 0;
}

int responder ( void* data )
{
	register uint64_t CACHE_ALIGNED transaction_id;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		respond( transaction_id );
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

	kthread_t* requester_thread = NULL;
	kthread_t* responder_thread = NULL;

	/**
	 * Shared memory region is 4mb, which is meant to fit into L1.
	 */
	cache = (cache_line_t*) kmalloc( 4*1024*1024, GFP_KERNEL );

	// Init Variables
	int i;
	for ( i = 0; i < transactions; ++i )
	{
		cache[i].regs[0] = MSG_AVAIL;
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
