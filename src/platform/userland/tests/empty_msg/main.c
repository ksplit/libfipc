/**
 * @File     : main.c
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This test sends an empty message in a round trip using the fipc library.
 *
 * NOTE: This program assumes an x86 architecture.
 */

#include "../libfipc_test.h"

#define TRANSACTIONS	1000
#define REQUESTER_CPU	1
#define RESPONDER_CPU	3
#define CHANNEL_ORDER	ilog2(sizeof(message_t))

// Thread Locks
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;

void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	// Wait until message is available (GetS)
	fipc_test_blocking_send_start( chan, &request );

	// Send request (GetM)
	fipc_send_msg_end ( chan, request );

	// Wait until message is received (GetS)
	fipc_test_blocking_recv_start( chan, &response );

	// Receive response (GetM)
	fipc_recv_msg_end( chan, response );
}

void respond ( header_t* chan )
{
	message_t* request;
	message_t* response;

	// Wait until request is received (GetS)
	fipc_test_blocking_recv_start( chan, &request );

	// Receive request (GetM)
	fipc_recv_msg_end( chan, request );

	// Wait until message is available (GetS)
	fipc_test_blocking_send_start( chan, &response );

	// Send response (GetM)
	fipc_send_msg_end( chan, response );
}

void* requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t* times = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	
	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	// Begin test
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		request( chan );

		end = RDTSCP();
		times[transaction_id] = end - start;
	}

	// End test
	fipc_test_stat_print_info( times, TRANSACTIONS );
	pthread_mutex_unlock( &requester_mutex );
	pthread_exit( 0 );
	return NULL;
}

void* responder ( void* data )
{
	uint64_t  transaction_id;
	header_t* chan = (header_t*) data;
	
	// Wait to begin test
	pthread_mutex_lock( &responder_mutex );


	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond( chan );
	}

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
