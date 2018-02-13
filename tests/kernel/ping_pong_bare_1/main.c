/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static
uint64_t about_x_cycles ( uint64_t x_cycles )
{
	uint64_t loop_iterations = ( x_cycles - 10 ) >> 1;
        uint64_t i;
        uint64_t t;
        uint64_t x = 1;
        uint64_t y = 1;
        asm volatile ("");
        for ( i = 0; i < loop_iterations; ++i )
        {
                t = y;
                y = x + y;
                x = t;
        }
        return y;
}

static inline
void request ( void )
{
	// Write Request
	line.regs[0] = 0;

	// Read Response
	while ( unlikely( line.regs[0] == 0 ) )
		fipc_test_pause();
}

static inline
uint64_t respond ( void )
{
	uint64_t ret;

	// Read Request
	while ( unlikely( line.regs[0] != 0 ) )
		fipc_test_pause();

	ret = about_x_cycles ( X_CYCLES );

	// Write Response
	line.regs[0] = ret;

	return ret;
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
	uint64_t sum = 0;
	start = RDTSC_START();
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		sum += about_x_cycles ( X_CYCLES );
	end = RDTSCP();

	pr_err ( "Busy Cycles: %llu\t%llu", ( end - start ) / transactions, sum );

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		start = RDTSC_START();

		request();

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
	uint64_t sum = 0;
	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		sum += respond();
	}

	pr_err( "Sum: %llu", sum );

	// End test
	fipc_test_thread_release_control_of_CPU();
	complete( &responder_comp );
	return 0;
}


int main ( void )
{
	line.regs[0] = 1;

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
