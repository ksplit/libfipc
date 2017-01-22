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
	int			ret;
	Header*		caller_header;
	Header*		callee_header;
	pthread_t*	caller_thread;
	pthread_t*	callee_thread;

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
	ret = test_fipc_create_channel( CHANNEL_ORDER,
									&caller_header, 
									&callee_header );

	if ( ret )
	{
		fprintf( stderr, "Error creating channel, ret = %d\n", ret );
		fipc_fini();
		return ret;
	}
	
	// Thread Init
	caller_thread = test_fipc_spawn_thread_with_channel( caller_header, 
														 &caller,
														 CALLER_CPU );
	if ( caller_thread == NULL )
	{
		fprintf(stderr, "Error setting up caller thread\n");
		test_fipc_free_channel(CHANNEL_ORDER, caller_header, callee_header);
		fipc_fini();
		return ret;
	}
	
	callee_thread = test_fipc_spawn_thread_with_channel( callee_header, 
														 &callee,
														 CALLEE_CPU );
	if ( callee_thread == NULL )
	{
		fprintf(stderr, "Error setting up caller thread\n");
		test_fipc_release_thread(caller_thread);
		test_fipc_free_channel(CHANNEL_ORDER, caller_header, callee_header);
		fipc_fini();
		return ret;
	}
	
	// Start the threads
	pthread_mutex_unlock( &callee_mutex );
	pthread_mutex_unlock( &caller_mutex );
	
	// Wait for one second, so we don't prematurely kill the caller or callee.
	sleep(1); 
	
	// Wait for them to complete, so we can tear things down
	ret = test_fipc_wait_for_thread(caller_thread);
	if ( ret )
	{
		fprintf(stderr, "Caller returned non-zero exit status %d\n", ret);
	}

	ret = test_fipc_wait_for_thread(callee_thread);
	if (ret)
	{
		fprintf(stderr, "Callee returned non-zero exit status %d\n", ret);
	}

	// Destruct
	test_fipc_free_channel(CHANNEL_ORDER, caller_header, callee_header);
	fipc_fini();
	return 0;
}

int main ( void )
{
	return doTest();
}
