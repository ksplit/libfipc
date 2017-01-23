/*
 * caller.c
 *
 * The "caller side" of the channel
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

	// Send Message
	ret = test_fipc_blocking_send_start( chan, &request );
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
	ret = test_fipc_blocking_recv_start( chan, &response );
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
	Header* chan = _caller_channel_header;
	unsigned long transaction_id;
	unsigned long start[TRANSACTIONS], end[TRANSACTIONS];
	uint64_t avgDelay = test_Average_Stopwatch_Delay();
	printf("AvgDelay: %lu\n", avgDelay);
	int ret = 0;

	pthread_mutex_lock( &caller_mutex );
	take_control_of_CPU();
	
	// Do recv/send
	printf("%s\n", "Roundtrip Times (in cycles):");
	
	for (transaction_id = 0; 
	     transaction_id < TRANSACTIONS;
	     transaction_id++)
	{
		start[transaction_id] = test_fipc_start_stopwatch();
		ret = do_one_msg(chan);
		end[transaction_id] = test_fipc_stop_stopwatch();

		if (ret)
		{
			fprintf(stderr, "error in send/recv, ret = %d, exiting...\n", ret);
			pthread_exit(&ret);
		}
	}

	
	for (transaction_id = 0;
             transaction_id < TRANSACTIONS;
             transaction_id++)
        {
		printf("\t%lu\n", end[transaction_id] - start[transaction_id] - avgDelay);
	}

	printf("%s\n", "Complete");

	release_control_of_CPU();
	pthread_mutex_unlock( &caller_mutex );
	pthread_exit(0);
}
