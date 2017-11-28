/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

int producer ( void* data )
{
	header_t**  chans = (header_t**) data;
	message_t* request;

	register uint64_t transaction_id;
	register uint64_t start;
	register uint64_t end;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI(ready_producers);

	while ( !test_ready )
		fipc_test_pause();
	
	fipc_test_mfence();

	start = RDTSC_START();

	int i = -1;

	// Produce
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		++i;
		if (i >= consumer_count) i = 0;

		if (fipc_send_msg_start( chans[i], &request ) != 0)
			continue;

		request->regs[0] = NULL_INVOCATION;
		fipc_send_msg_end( chans[i], request );
	}
	
	end = RDTSCP();

	// End test
	pr_err( "Producer completed in %llu, and the average was %llu.\n", end - start, (end - start) / transactions );
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI(completed_producers);
	return 0;
}

int consumer ( void* data )
{
	header_t**  chans = (header_t**) data;
	message_t* request;

	int halt = 0;
	int i = -1;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();
	
	fipc_test_mfence();

	// Consume
	while ( !halt )
	{
		++i;
		if ( i >= producer_count ) i = 0;
		// Poll Channels in round robin fashion
	//	while ( ! fipc_recv_msg_start( chans[i % producer_count], &request ) )
	//		++i;

	 //	if (request->regs[0] == HALT) halt = 1;
	 //	fipc_recv_msg_end( chans[i % producer_count], request );
		if (fipc_recv_msg_start( chans[i], &request) != 0)
			continue;

		if (request->regs[0] == HALT) halt = 1;
	 	fipc_recv_msg_end( chans[i], request );
	}

	// End test
	fipc_test_mfence();
	pr_err("CONSUMER FINISHING\n");
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI( completed_consumers );
	return 0;
}

int controller ( void* data )
{
	int i;
	int j;
	message_t* haltMsg;

	// FIPC Init
	fipc_init();

	header_t***  prod_headers = (header_t***) vmalloc( producer_count*sizeof(header_t**) );
	header_t***  cons_headers = (header_t***) vmalloc( consumer_count*sizeof(header_t**) );

	for ( i = 0; i < producer_count; ++i )
		prod_headers[i] = (header_t**) vmalloc( consumer_count*sizeof(header_t*) );

	for ( i = 0; i < consumer_count; ++i )
		cons_headers[i] = (header_t**) vmalloc( producer_count*sizeof(header_t*) );

	for ( i = 0; i < producer_count; ++i )
	{
		for ( j = 0; j < consumer_count; ++j )
		{
			fipc_test_create_channel( CHANNEL_ORDER, &prod_headers[i][j], &cons_headers[j][i] );

			if ( prod_headers[i][j] == NULL || cons_headers[j][i] == NULL )
			{
				pr_err( "%s\n", "Error while creating channel" );
				return -1;
			}
		}
	}

	// Thread Allocation
	kthread_t** cons_threads = (kthread_t**) vmalloc( consumer_count*sizeof(kthread_t*) );
	kthread_t** prod_threads = NULL;
	
	if ( producer_count >= 2 )
		prod_threads = (kthread_t**) vmalloc( (producer_count-1)*sizeof(kthread_t*) );

	// Spawn Threads
	for ( i = 0; i < (producer_count-1); ++i )
	{
		prod_threads[i] = fipc_test_thread_spawn_on_CPU ( producer, prod_headers[i], producer_cpus[i] );

		if ( prod_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}

	for ( i = 0; i < consumer_count; ++i )
	{
		cons_threads[i] = fipc_test_thread_spawn_on_CPU ( consumer, cons_headers[i], consumer_cpus[i] );
		
		if ( cons_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}
	
	// Start threads
	for ( i = 0; i < (producer_count-1); ++i )
		wake_up_process( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		wake_up_process( cons_threads[i] );

	
	// Wait for threads to be ready for test
	while ( ready_consumers < consumer_count )
		fipc_test_pause();

	while ( ready_producers < (producer_count-1) )
		fipc_test_pause();

	fipc_test_mfence();

	// Begin Test
	test_ready = 1;

	fipc_test_sfence();

	// This thread is also a producer
	producer( prod_headers[producer_count - 1] );

	// Wait for producers to complete
	while ( completed_producers < producer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Tell consumers to halt
	for ( i = 0; i < consumer_count; ++i )
	{
		fipc_test_blocking_send_start( prod_headers[producer_count - 1][i], &haltMsg );
		haltMsg->regs[0] = HALT;
		fipc_send_msg_end ( prod_headers[producer_count - 1][i], haltMsg );
	}

	// Wait for consumers to complete
	while ( completed_consumers < consumer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Clean up
	for ( i = 0; i < (producer_count-1); ++i )
		fipc_test_thread_free_thread( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		fipc_test_thread_free_thread( cons_threads[i] );

	if ( prod_threads != NULL )
		vfree( prod_threads );

	vfree( cons_threads );

	for ( i = 0; i < producer_count; ++i )
	{
		for ( j = 0; j < consumer_count; ++j )
		{
			fipc_test_free_channel( CHANNEL_ORDER, prod_headers[i][j], cons_headers[j][i] );
		}
	}

	for ( i = 0; i < consumer_count; ++i )
		vfree( cons_headers[i] );

	for ( i = 0; i < producer_count; ++i )
		vfree( prod_headers[i] );

	vfree( cons_headers );
	vfree( prod_headers );

	fipc_fini();

	// End Experiment
	fipc_test_mfence();
	test_finished = 1;
	return 0;
}

int init_module(void)
{
	kthread_t* controller_thread = fipc_test_thread_spawn_on_CPU ( controller, NULL, producer_cpus[producer_count-1] );

	if ( controller_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}

	wake_up_process( controller_thread );

	while ( !test_finished )
		fipc_test_pause();

	fipc_test_mfence();
	fipc_test_thread_free_thread( controller_thread );
	pr_err("finished\n");

	return 0;

}

void cleanup_module(void)
{
	return;
}

MODULE_LICENSE("GPL");
