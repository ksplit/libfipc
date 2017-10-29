/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include "test.h"

static inline
void load ( uint64_t index )
{
	while ( unlikely( cache[index].regs[0] != MSG_READY ) )
		fipc_test_pause();
}

static inline
void stage ( uint64_t index )
{
	cache[index].regs[0] = MSG_READY;
}

void* loader ( void* data )
{
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t CACHE_ALIGNED correction = fipc_test_time_get_correction();
	register int64_t* CACHE_ALIGNED times = malloc( transactions * sizeof( int64_t ) );

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	while ( testready == 0 )
		fipc_test_pause();

	// Wait for stager to complete
	load( transactions );
	fipc_test_mfence();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		start = RDTSC_START();

		load( load_order[transaction_id] );
		fipc_test_mfence();

		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
	}

	printf( "Correction: %lu\n", correction );

	// End test
	fipc_test_thread_release_control_of_CPU();
	fipc_test_stat_print_raw( times, transactions, transactions );
	fipc_test_stat_get_and_print_stats( times, transactions );
	free( times );
	pthread_exit( 0 );
	return NULL;
}

void* stager ( void* data )
{
	register uint64_t CACHE_ALIGNED transaction_id;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	while( testready == 0 )
		fipc_test_pause();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		stage( transaction_id );
	}

	stage( transactions ); // Used a flag to start loader

	// End test
	fipc_test_thread_release_control_of_CPU();
	pthread_exit( 0 );
	return NULL;
}


int main ( void )
{
	pthread_t* loader_thread = NULL;
	pthread_t* stager_thread = NULL;

	/**
	 * Shared memory region is 16kb, which is meant to fit into L1.
	 */
	cache = (cache_line_t*) malloc( 16*1024 + 64 );

	// Create Threads
	loader_thread = fipc_test_thread_spawn_on_CPU ( loader, NULL, loader_cpu );
	if ( loader_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	stager_thread = fipc_test_thread_spawn_on_CPU ( stager, NULL, stager_cpu );
	if ( stager_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// Start threads
	testready = 1;
	
	// Wait for thread completion
	fipc_test_thread_wait_for_thread( loader_thread );
	fipc_test_thread_wait_for_thread( stager_thread );
	
	// Clean up
	fipc_test_thread_free_thread( loader_thread );
	fipc_test_thread_free_thread( stager_thread );
	pthread_exit( NULL );
	return 0;
}
