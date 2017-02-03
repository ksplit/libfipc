/*
 * main.c
 *
 * Contains init/exit for empty_msg test
 *
 * See README.txt for more details.
 */

#include "empty_msg.h"

#define CALLER_CPU 1
#define CALLEE_CPU 3
/* channel buffers contain just one message slot */
#define CHANNEL_ORDER ilog2(sizeof(struct fipc_message))

static int doTest( void )
{
	int        ret;
	header_t*  caller_header;
	header_t*  callee_header;
	pthread_t* caller_thread;
	pthread_t* callee_thread;

	// Lock Init
	pthread_mutex_lock( &callee_mutex );
	pthread_mutex_lock( &caller_mutex );

	// FIPC Init
	ret = fipc_init();
	if ( ret )
	{
		fprintf( stderr, "Error init'ing libfipc, ret = %d\n", ret );
		return ret;
	}
	
	// Channel Init
	ret = fipc_test_create_channel( CHANNEL_ORDER,
									&caller_header, 
									&callee_header );

	if ( ret )
	{
		fprintf( stderr, "Error creating channel, ret = %d\n", ret );
		fipc_fini();
		return ret;
	}
	
	// Thread Init
	caller_thread = fipc_test_thread_spawn_on_CPU( &caller, 
													caller_header,
													CALLER_CPU );
	if ( caller_thread == NULL )
	{
		fprintf( stderr, "Error setting up caller thread\n" );
		fipc_test_free_channel( CHANNEL_ORDER, caller_header, callee_header );
		fipc_fini();
		return ret;
	}
	
	callee_thread = fipc_test_thread_spawn_on_CPU( &callee,
													callee_header, 
													CALLEE_CPU );
	if ( callee_thread == NULL )
	{
		fprintf( stderr, "Error setting up caller thread\n" );
		fipc_test_thread_release_thread(caller_thread);
		fipc_test_thread_free_thread(caller_thread);
		fipc_test_free_channel( CHANNEL_ORDER, caller_header, callee_header );
		fipc_fini();
		return ret;
	}
	
	// Start the threads
	pthread_mutex_unlock( &callee_mutex );
	
	// Wait for one second, so we don't prematurely kill the caller or callee.
	sleep(1); 
	
	// Wait for them to complete, so we can tear things down
	ret = fipc_test_thread_wait_for_thread( caller_thread );
	if ( ret )
	{
		fprintf( stderr, "Caller returned non-zero exit status %d\n", ret );
	}

	ret = fipc_test_thread_wait_for_thread( callee_thread );
	if (ret)
	{
		fprintf( stderr, "Callee returned non-zero exit status %d\n", ret );
	}

	// Destruct
	fipc_test_free_channel( CHANNEL_ORDER, caller_header, callee_header );
	fipc_test_thread_free_thread(callee_thread);
	fipc_test_thread_free_thread(caller_thread);
	fipc_fini();
	return 0;
}

int main ( void )
{
	return doTest();
}
