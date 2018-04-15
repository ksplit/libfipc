/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include "test.h"


static inline
void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	int j;
	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );

		// Marshalling
		request->flags = MARSHALL_ORDER;
		for ( j = 0; j < MARSHALL_ORDER; ++j )
			request->regs[j] = j;

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
	uint64_t   answer;

	int i;
	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_recv_start( chan, &request );

		// Dispatch loop
		switch ( request->flags )
		{
			case 0:
				answer = null_invocation();
				break;

			case 1:
				answer = increment( request->regs[0] );
				break;

			case 2:
				answer = add_2_nums( request->regs[0], request->regs[1] );
				break;

			case 3:
				answer = add_3_nums( request->regs[0], request->regs[1], request->regs[2] );
				break;

			case 4:
				answer = add_4_nums( request->regs[0], request->regs[1], request->regs[2], request->regs[3] );
				break;
				
			case 5:
				answer = add_5_nums( request->regs[0], request->regs[1], request->regs[2], request->regs[3], request->regs[4] );
				break;
				
			case 6:
				answer = add_6_nums( request->regs[0], request->regs[1], request->regs[2], request->regs[3], request->regs[4], request->regs[5] );
				break;
				
			case 7:
				answer = add_7_nums( request->regs[0], request->regs[1], request->regs[2], request->regs[3], request->regs[4], request->regs[5], request->regs[6] );
				break;
		}

		fipc_recv_msg_end( chan, request );
	}

	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_send_start( chan, &response );
		response->regs[0] = answer;
		fipc_send_msg_end( chan, response );
	}
}

void* requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t CACHE_ALIGNED correction = fipc_test_time_get_correction();
	register int64_t* CACHE_ALIGNED times = malloc( TRANSACTIONS * sizeof( int64_t ) );
	
	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );
	
	// Begin test
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		request( chan );

		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
	}

	// End test
	fipc_test_stat_get_and_print_stats( times, TRANSACTIONS );
	free( times );
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

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
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
