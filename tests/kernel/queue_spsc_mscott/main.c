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
	int i = -1;
	int rank = *(int*)data;

	node_t*   t = node_table[rank];
	queue_t** q = prod_queue[rank];

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

	for ( transaction_id = 0; transaction_id < transactions; )
	{
		t[transaction_id].data = NULL_INVOCATION;

		++i; if (i >= consumer_count) i = 0;

		if ( enqueue( q[i], &t[transaction_id] ) == SUCCESS )
		{
			transaction_id++;
		}
	}
	
	end = RDTSCP();

	// End test
	pr_err( "Cycles per message %llu\n", (end - start) / transactions );
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI(completed_producers);
	return 0;
}

int consumer ( void* data )
{
	int i = -1;
	int rank = *(int*)data;

	queue_t** q = cons_queue[rank];

	data_t d;
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
		++i; if ( i >= producer_count ) i = 0;

		// Receive and unmarshall d
		if ( dequeue( q[i], &d ) == SUCCESS )
		{
			// Process Request
			switch ( d )
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
	int j;

	// Queue Allocation
	queue_t* queues = (queue_t*) vmalloc( producer_count*consumer_count*sizeof(queue_t) );
	
	for ( i = 0; i < producer_count*consumer_count; ++i )
		init_queue ( &queues[i] );

	prod_queue = (queue_t***) vmalloc( producer_count*sizeof(queue_t**) );
	cons_queue = (queue_t***) vmalloc( consumer_count*sizeof(queue_t**) );

	for ( i = 0; i < producer_count; ++i )
		prod_queue[i] = (queue_t**) vmalloc( consumer_count*sizeof(queue_t*) );

	for ( i = 0; i < consumer_count; ++i )
		cons_queue[i] = (queue_t**) vmalloc( producer_count*sizeof(queue_t*) );

	for ( i = 0; i < producer_count; ++i )
	{
		for ( j = 0; j < consumer_count; ++j )
		{
			prod_queue[i][j] = &queues[i*producer_count + j];
			cons_queue[j][i] = &queues[i*producer_count + j];
		}
	}

	// Node Table Allocation
	node_table = (node_t**) vmalloc( producer_count*sizeof(node_t*) );

	for ( i = 0; i < producer_count; ++i )
		node_table[i] = (node_t*) vmalloc( transactions*sizeof(node_t) );

	node_t* haltMsg = (node_t*) vmalloc( consumer_count*sizeof(node_t) );

	fipc_test_mfence();
	
	// Thread Allocation
	kthread_t** cons_threads = (kthread_t**) vmalloc( consumer_count*sizeof(kthread_t*) );
	kthread_t** prod_threads = NULL;
	
	if ( producer_count >= 2 )
		prod_threads = (kthread_t**) vmalloc( (producer_count-1)*sizeof(kthread_t*) );

	int* p_rank = (int*) vmalloc( (producer_count-1)*sizeof(int) );
	int* c_rank = (int*) vmalloc( (consumer_count)*sizeof(int) );

	// Spawn Threads
	for ( i = 0; i < (producer_count-1); ++i )
	{
		p_rank[i] = i;
		prod_threads[i] = fipc_test_thread_spawn_on_CPU ( producer, &p_rank[i], producer_cpus[i] );

		if ( prod_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}

	for ( i = 0; i < consumer_count; ++i )
	{
		c_rank[i] = i;
		cons_threads[i] = fipc_test_thread_spawn_on_CPU ( consumer, &c_rank[i], consumer_cpus[i] );
		
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

	fipc_test_mfence();

	// This thread is also a producer
	int p = producer_count-1;
	producer( &p );

	// Wait for producers to complete
	while ( completed_producers < producer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Tell consumers to halt
	for ( i = 0; i < consumer_count; ++i )
	{
		haltMsg[i].data = HALT;

		enqueue( prod_queue[p][i], &haltMsg[i] );
	}
	
	// Wait for consumers to complete
	while ( completed_consumers < consumer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Clean up
	vfree( c_rank );
	vfree( p_rank );

	for ( i = 0; i < consumer_count; ++i )
		fipc_test_thread_free_thread( cons_threads[i] );

	for ( i = 0; i < (producer_count-1); ++i )
		fipc_test_thread_free_thread( prod_threads[i] );

	vfree( cons_threads );

	if ( prod_threads != NULL )
		vfree( prod_threads );

	vfree( haltMsg );

	for ( i = 0; i < producer_count; ++i )
		vfree( node_table[i] );

	vfree( node_table );

	for ( i = 0; i < consumer_count; ++i )
		vfree( cons_queue[i] );

	for ( i = 0; i < producer_count; ++i )
		vfree( prod_queue[i] );

	vfree( cons_queue );
	vfree( prod_queue );

	for ( i = 0; i < producer_count*consumer_count; ++i )
		free_queue( &queues[i] );

	vfree( queues );

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
