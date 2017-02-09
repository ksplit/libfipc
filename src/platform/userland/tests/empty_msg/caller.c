/*
 * caller.c
 *
 * The "caller side" of the channel
 *
 * Copyright: University of Utah
 */

#include "empty_msg.h"

static inline
int do_one_msg ( header_t* chan )
{
	int ret;
	message_t* request;
	message_t* response;

	// Send Message
	ret = fipc_test_blocking_send_start( chan, &request );
	if ( ret )
	{
		return ret;
	}
	printf("MESSAGE: %p\n", request);
	ret = fipc_send_msg_end ( chan, request );
	if ( ret )
	{
		return ret;
	}
	
	// Receive Response
	ret = fipc_test_blocking_recv_start( chan, &response );
	if ( ret )
	{
		return ret;
	}
	
	printf("MESSAGE: %p\n", response);
	ret = fipc_recv_msg_end( chan, response );
	if ( ret )
	{
		return ret;
	}

	return 0;
}

void* caller(void *_caller_channel_header)
{
	header_t* chan = _caller_channel_header;
	uint32_t  transaction_id;
	uint64_t  start;
	uint64_t  end;
	float     sum = 0;
	int       ret = 0;

	pthread_mutex_lock( &caller_mutex );
	fipc_test_thread_take_control_of_CPU();
	
	// Do recv/send
	printf("%s\n", "Roundtrip Times (in cycles):");
	
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = fipc_test_time_get_timestamp_sf();
		ret = do_one_msg(chan);
		end = fipc_test_time_get_timestamp_mf();

		sum += end - start;

		if (ret)
		{
			fprintf(stderr, "error in send/recv, ret = %d, exiting...\n", ret);
			pthread_exit(&ret);
		}
	}
	
	printf( "Average Round Trip Cycles:\t%f\n", sum / TRANSACTIONS );

	fipc_test_thread_release_control_of_CPU();
	pthread_mutex_unlock( &caller_mutex );
	pthread_exit(0);
}
