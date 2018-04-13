/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include "test.h"
#define FINE_GRAINED

static int queue_depth = 1;

void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;

	for ( i = 0; i < queue_depth; ++i ) {
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}
}

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

void* requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;
#if defined(FINE_GRAINED)
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register int64_t* CACHE_ALIGNED times = malloc( transactions * sizeof( int64_t ) );
	register uint64_t CACHE_ALIGNED correction = fipc_test_time_get_correction();
#endif
	register uint64_t CACHE_ALIGNED whole_start;
	register uint64_t CACHE_ALIGNED whole_end;

	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif

		request( chan );
#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();

	// End test
#if defined(FINE_GRAINED) 
	fipc_test_stat_get_and_print_stats( times, transactions );
	printf("Correction value: %llu\n", correction);
  	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		printf("%llu\n", times[transaction_id]);
	}

	free( times );
#endif
	// Print count
	printf("-------------------------------------------------\n");
	
	printf("Average across entire loop: %llu\n", 
			(whole_end - whole_start) / transactions);

	pthread_mutex_unlock( &requester_mutex );
	pthread_exit( 0 );
	return NULL;
}

void* responder ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &responder_mutex );

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		respond( chan );
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
	
	header_t* requester_header = NULL;
	header_t* responder_header = NULL;

	fipc_init();
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header, &responder_header );

	if ( requester_header == NULL || responder_header == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating channel" );
		return -1;
	}

	// Create Threads
	pthread_t* requester_thread = fipc_test_thread_spawn_on_CPU ( requester, requester_header, REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	pthread_t* responder_thread = fipc_test_thread_spawn_on_CPU ( responder, responder_header, RESPONDER_CPU );
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
	fipc_test_free_channel( CHANNEL_ORDER, requester_header, responder_header );
	fipc_fini();
	pthread_exit( NULL );
	return 0;
}
