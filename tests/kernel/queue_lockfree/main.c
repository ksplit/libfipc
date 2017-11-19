/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

int noinline null_invocation ( void )
{
	asm volatile ("");
	return 0;
}

int producer ( void* data )
{
	queue_t*   q = &queue;
	request_t* t = (request_t*) data;

	register uint64_t transaction_id;
	register uint64_t start;
	register uint64_t end;

	// Touching data
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		t[transaction_id].data = 0;
		t[transaction_id].next = NULL;
	}

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI(ready_producers);

	while ( !test_ready )
		fipc_test_pause();
	
	fipc_test_mfence();

	start = RDTSC_START();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		t[transaction_id].data = NULL_INVOCATION;

		enqueue( q, &t[transaction_id] );
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
	queue_t* q = (queue_t*) data;

	request_t* request = NULL;

	int halt = 0;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	while ( !halt )
	{
		// Receive and unmarshall request
		dequeue( q, &request );

		if ( request != NULL )
		{
			// Process Request
			switch ( request->data )
			{
				case NULL_INVOCATION:
					null_invocation();
					break;

				case HALT:
					halt = 1;
					break;
			}
		}
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

	// Queue Init
	init_queue ( &queue );

	// Node Table Allocation
	request_t** node_table = (request_t**) vmalloc( producer_count*sizeof(request_t*) );

	for ( i = 0; i < producer_count; ++i )
		node_table[i] = (request_t*) vmalloc( transactions*sizeof(request_t) );

	request_t* haltMsg = (request_t*) vmalloc( consumer_count*sizeof(request_t) );
	
	// Thread Allocation
	kthread_t** cons_threads = (kthread_t**) vmalloc( consumer_count*sizeof(kthread_t*) );
	kthread_t** prod_threads = NULL;
	
	if ( producer_count >= 2 )
		prod_threads = (kthread_t**) vmalloc( (producer_count-1)*sizeof(kthread_t*) );

	// Spawn Threads
	for ( i = 0; i < (producer_count-1); ++i )
	{
		prod_threads[i] = fipc_test_thread_spawn_on_CPU ( producer, node_table[i], producer_cpus[i] );

		if ( prod_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}

	for ( i = 0; i < consumer_count; ++i )
	{
		cons_threads[i] = fipc_test_thread_spawn_on_CPU ( consumer, &queue, consumer_cpus[i] );
		
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

	// This thread is also a producer
	producer( node_table[producer_count-1] );

	// Wait for producers to complete
	while ( completed_producers < producer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Tell consumers to halt
	for ( i = 0; i < consumer_count; ++i )
	{
		haltMsg[i].next = 0;
		haltMsg[i].data = HALT;

		enqueue( &queue, &haltMsg[i] );
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

	for ( i = 0; i < producer_count; ++i )
		vfree( node_table[i] );

	if ( prod_threads != NULL )
		vfree( prod_threads );

	vfree( cons_threads );
	vfree( node_table );
	vfree( haltMsg );
	free_queue( &queue );

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
