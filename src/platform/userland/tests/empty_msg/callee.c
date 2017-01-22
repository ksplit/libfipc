/*
 * callee.c
 *
 * Code for the "callee side" of the channel
 *
 * Copyright: University of Utah
 */

#include "empty_msg.h"

static inline
int do_one_msg ( Header* chan )
{
	int ret;
	Message* request;
	Message* response;
	
	// Receive empty request
	ret = test_fipc_blocking_recv_start( chan, &request );
	if ( ret )
	{
		return ret;
	}
	
	ret = fipc_recv_msg_end( chan, request );
	if ( ret )
	{
		return ret;
	}
	
	// Send empty response
	ret = test_fipc_blocking_send_start( chan, &response );
	if ( ret )
	{
		return ret;
	}
	
	ret = fipc_send_msg_end( chan, response );
	if ( ret )
	{
		return ret;
	}

	return 0;
}

void* callee( void* _callee_channel_header )
{
    Header* chan = _callee_channel_header;
	unsigned long transaction_id;
	int ret = 0;
	
	pthread_mutex_lock( &callee_mutex );
	take_control_of_CPU();
	
	// Do recv/send
	for (transaction_id = 0; 
	     transaction_id < TRANSACTIONS;
	     transaction_id++)
	{
		ret = do_one_msg(chan);
		if (ret)
		{
			fprintf(stderr, "error in send/recv, ret = %d, exiting...\n", ret);
			pthread_exit(&ret);
		}
	}

	release_control_of_CPU();
	pthread_mutex_unlock( &callee_mutex );
	pthread_exit(0);
}
