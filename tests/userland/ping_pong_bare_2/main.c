/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include "test.h"

static inline void request ( void )
{
	// Wait until message is available (GetS)
	// Send request (GetM)
	req_line = req_sequence;

	// Wait until message is received (GetS)
	while ( likely(resp_line != req_sequence) )
		fipc_test_pause();

	req_sequence ++;
}

static inline void respond ( void )
{
	// Wait until message is received (GetS)
	while ( likely(req_line != resp_sequence ))
		fipc_test_pause();

	// Send response (GetM)
	resp_line = resp_sequence;
	resp_sequence ++;
}
void* requester ( void* data )
{
	register uint64_t  CACHE_ALIGNED transaction_id;
	register uint64_t  CACHE_ALIGNED start;
	register uint64_t  CACHE_ALIGNED end;
	register uint64_t* CACHE_ALIGNED times = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	
	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );


	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		request();

		end = RDTSCP();
		times[transaction_id] = end - start;
	}

	// End test
	fipc_test_stat_print_info( times, TRANSACTIONS );
	free( times );
	pthread_mutex_unlock( &requester_mutex );
	pthread_exit( 0 );
	return NULL;
}

void* responder ( void* data )
{
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &responder_mutex );

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond();
	}

	// End test
	pthread_mutex_unlock( &responder_mutex );
	pthread_exit( 0 );
	return NULL;
}


int main ( void )
{
	// Begin critical section
	pthread_mutex_init( &requester_mutex, NULL );
	pthread_mutex_init( &responder_mutex, NULL );
	
	pthread_mutex_lock( &requester_mutex );
	pthread_mutex_lock( &responder_mutex );
	
	// Create Threads
	pthread_t* requester_thread = fipc_test_thread_spawn_on_CPU ( requester,
																NULL,
																REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	pthread_t* responder_thread = fipc_test_thread_spawn_on_CPU ( responder,
																NULL,
																RESPONDER_CPU );
	if ( responder_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// End critical section = start threads
	pthread_mutex_unlock( &responder_mutex );
	pthread_mutex_unlock( &requester_mutex );
	
	// Wait for thread completion
	fipc_test_thread_wait_for_thread( requester_thread );
	fipc_test_thread_wait_for_thread( responder_thread );
	
	// Clean up
	fipc_test_thread_free_thread( requester_thread );
	fipc_test_thread_free_thread( responder_thread );
	pthread_exit( NULL );
	return 0;
}
