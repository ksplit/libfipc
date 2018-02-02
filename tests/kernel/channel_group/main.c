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

uint64_t about_200_cycles ( void )
{
	uint64_t i;
	uint64_t t;
	uint64_t x = 1;
	uint64_t y = 1;
	asm volatile ("");
	for ( i = 0; i < 90; ++i )
	{
		t = y;
		y = x + y;
		x = t;
	}
	return y;
}

uint64_t about_1000_cycles ( void )
{
	uint64_t i;
	uint64_t t;
	uint64_t x = 1;
	uint64_t y = 1;
	asm volatile ("");
	for ( i = 0; i < 485; ++i )
	{
		t = y;
		y = x + y;
		x = t;
	}
	return y;
}

uint64_t about_2000_cycles ( void )
{
	uint64_t i;
	uint64_t t;
	uint64_t x = 1;
	uint64_t y = 1;
	asm volatile ("");
	for ( i = 0; i < 970; ++i )
	{
		t = y;
		y = x + y;
		x = t;
	}

	return y;
}

int producer ( void* data )
{
	uint64_t transaction_id;
	uint64_t start;
	uint64_t end;
	uint64_t i = 0;

	uint64_t rank = *(uint64_t*)data;
	node_t*   t = node_tables[rank];
	queue_t** q = prod_queues[rank];

	// Touching data
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		t[transaction_id].regs[0] = 0;
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
		t[transaction_id].regs[0] = NULL_INVOCATION;

		if ( enqueue_forw( q[i], &t[transaction_id] ) == SUCCESS )
		{
			transaction_id++;
		}

		++i; if (i >= consumer_count) i = 0;
	}

	end = RDTSCP();

	// End test
	pr_err( "Producer %llu finished. Cycles per message %llu\n", rank, (end - start) / transactions );
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI(completed_producers);
	return 0;
}

int consumer ( void* data )
{
	uint64_t start;
	uint64_t end;
	uint64_t i    = 0;
	uint64_t halt = 0;
	data_t   d;

	uint64_t rank = *(uint64_t*)data;
	queue_t** q = cons_queues[rank];

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();

	while ( !halt )
	{
		// Receive and unmarshall d
		if ( dequeue_forw( q[i], &d ) == SUCCESS )
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

		++i; if ( i >= producer_count ) i = 0;
	}

	end = RDTSCP();

	// End test
	fipc_test_mfence();
	pr_err( "Consumer %llu finished. Cycles per message %llu\n", rank, (end - start) / transactions );
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI( completed_consumers );
	return 0;
}


int controller ( void* data )
{
	uint64_t transaction_id;
	uint64_t start;
	uint64_t end;
	uint64_t sum = 0;
	start = RDTSC_START();

	for ( transaction_id = 0; transaction_id < 1000; ++transaction_id )
	{
		sum += about_200_cycles();
	}

	end = RDTSCP();

	pr_err( "%llu\t%llu\n", ( end - start ) / 1000, sum );
	// uint64_t i;
	// uint64_t j;

	// // Queue Allocation
	// queue_t* queues = (queue_t*) vmalloc( producer_count*consumer_count*sizeof(queue_t) );

	// for ( i = 0; i < producer_count*consumer_count; ++i )
	// 	init_queue ( &queues[i] );

	// prod_queues = (queue_t***) vmalloc( producer_count*sizeof(queue_t**) );
	// cons_queues = (queue_t***) vmalloc( consumer_count*sizeof(queue_t**) );

	// for ( i = 0; i < producer_count; ++i )
	// 	prod_queues[i] = (queue_t**) vmalloc( consumer_count*sizeof(queue_t*) );

	// for ( i = 0; i < consumer_count; ++i )
	// 	cons_queues[i] = (queue_t**) vmalloc( producer_count*sizeof(queue_t*) );

	// // Queue Linking
	// for ( i = 0; i < producer_count; ++i )
	// {
	// 	for ( j = 0; j < consumer_count; ++j )
	// 	{
	// 		prod_queues[i][j] = &queues[i*producer_count + j];
	// 		cons_queues[j][i] = &queues[i*producer_count + j];
	// 	}
	// }

	// // Node Table Allocation
	// node_tables = (node_t**) vmalloc( producer_count*sizeof(node_t*) );

	// for ( i = 0; i < producer_count; ++i )
	// 	node_tables[i] = (node_t*) vmalloc( transactions*sizeof(node_t) );

	// node_t* haltMsg = (node_t*) vmalloc( consumer_count*sizeof(node_t) );

	// fipc_test_mfence();

	// // Thread Allocation
	// kthread_t** cons_threads = (kthread_t**) vmalloc( consumer_count*sizeof(kthread_t*) );
	// kthread_t** prod_threads = NULL;

	// if ( producer_count >= 2 )
	// 	prod_threads = (kthread_t**) vmalloc( (producer_count-1)*sizeof(kthread_t*) );

	// uint64_t* p_rank = (uint64_t*) vmalloc( producer_count*sizeof(uint64_t) );
	// uint64_t* c_rank = (uint64_t*) vmalloc( consumer_count*sizeof(uint64_t) );

	// // Spawn Threads
	// for ( i = 0; i < (producer_count-1); ++i )
	// {
	// 	p_rank[i] = i;
	// 	prod_threads[i] = fipc_test_thread_spawn_on_CPU ( producer, &p_rank[i], producer_cpus[i] );

	// 	if ( prod_threads[i] == NULL )
	// 	{
	// 		pr_err( "%s\n", "Error while creating thread" );
	// 		return -1;
	// 	}
	// }

	// for ( i = 0; i < consumer_count; ++i )
	// {
	// 	c_rank[i] = i;
	// 	cons_threads[i] = fipc_test_thread_spawn_on_CPU ( consumer, &c_rank[i], consumer_cpus[i] );

	// 	if ( cons_threads[i] == NULL )
	// 	{
	// 		pr_err( "%s\n", "Error while creating thread" );
	// 		return -1;
	// 	}
	// }

	// // Start threads
	// for ( i = 0; i < (producer_count-1); ++i )
	// 	wake_up_process( prod_threads[i] );

	// for ( i = 0; i < consumer_count; ++i )
	// 	wake_up_process( cons_threads[i] );

	// // Wait for threads to be ready for test
	// while ( ready_consumers < consumer_count )
	// 	fipc_test_pause();

	// while ( ready_producers < (producer_count-1) )
	// 	fipc_test_pause();

	// fipc_test_mfence();

	// // Begin Test
	// test_ready = 1;

	// fipc_test_mfence();

	// // This thread is also a producer
	// p_rank[producer_count-1] = producer_count-1;
	// producer( &p_rank[producer_count-1] );

	// // Wait for producers to complete
	// while ( completed_producers < producer_count )
	// 	fipc_test_pause();

	// fipc_test_mfence();

	// // Tell consumers to halt
	// for ( i = 0; i < consumer_count; ++i )
	// {
	// 	haltMsg[i].regs[0] = HALT;

	// 	enqueue_forw( prod_queues[producer_count-1][i], &haltMsg[i] );
	// }

	// // Wait for consumers to complete
	// while ( completed_consumers < consumer_count )
	// 	fipc_test_pause();

	// fipc_test_mfence();

	// // Clean up
	// vfree( c_rank );
	// vfree( p_rank );

	// for ( i = 0; i < consumer_count; ++i )
	// 	fipc_test_thread_free_thread( cons_threads[i] );

	// for ( i = 0; i < (producer_count-1); ++i )
	// 	fipc_test_thread_free_thread( prod_threads[i] );

	// vfree( cons_threads );

	// if ( prod_threads != NULL )
	// 	vfree( prod_threads );

	// vfree( haltMsg );

	// for ( i = 0; i < producer_count; ++i )
	// 	vfree( node_tables[i] );

	// vfree( node_tables );

	// for ( i = 0; i < consumer_count; ++i )
	// 	vfree( cons_queues[i] );

	// for ( i = 0; i < producer_count; ++i )
	// 	vfree( prod_queues[i] );

	// vfree( cons_queues );
	// vfree( prod_queues );

	// for ( i = 0; i < producer_count*consumer_count; ++i )
	// 	free_queue( &queues[i] );

	// vfree( queues );

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
	pr_err("Test finished\n");

	return 0;
}

void cleanup_module(void)
{
	return;
}

MODULE_LICENSE("GPL");
