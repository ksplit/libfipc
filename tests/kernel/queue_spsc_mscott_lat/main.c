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

int noinline add_6_nums ( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f )
{
	asm volatile ("");
	return a + b + c + d + e + f;
}

int producer ( void* data )
{
	uint64_t transaction_id;
	uint64_t start;
	uint64_t end;
	uint64_t i = 0;

	uint64_t  rank = *(uint64_t*)data;
	queue_t** qf   = prod_queues_forw[rank];
	queue_t** qb   = prod_queues_back[rank];
	node_t*   t    = node_tables[rank];

	// Message Data
	uint64_t a;
	uint64_t b;
	uint64_t c;
	uint64_t d;
	uint64_t e;
	uint64_t f;
	uint64_t g;

	// Touching data
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
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
		t[transaction_id].a = ADD_6_NUMS;
		t[transaction_id].b = 1;
		t[transaction_id].c = 2;
		t[transaction_id].d = 3;
		t[transaction_id].e = 4;
		t[transaction_id].f = 5;
		t[transaction_id].g = 6;

		if ( enqueue( qf[i], &t[transaction_id] ) == SUCCESS )
		{
			transaction_id++;

			while ( dequeue( qb[i], &a, &b, &c, &d, &e, &f, &g ) != SUCCESS )
				fipc_test_pause();
		}

		++i; if (i >= consumer_count) i = 0;
	}

	end = RDTSCP();

	test_results[rank] = ((end - start) / transactions) / 2;
	fipc_test_mfence();

	// End test
	pr_err( "Producer %llu finished.\n", rank );
	fipc_test_FAI(completed_producers);
	return 0;
}

int consumer ( void* data )
{
	uint64_t i    = 0;
	uint64_t halt = 0;

	uint64_t  rank = *(uint64_t*)data;
	queue_t** qf   = cons_queues_forw[rank];
	queue_t** qb   = prod_queues_back[rank];

	// Message Data
	uint64_t a;
	uint64_t b;
	uint64_t c;
	uint64_t d;
	uint64_t e;
	uint64_t f;
	uint64_t g;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	while ( !halt )
	{
		// Receive and unmarshall d
		if ( dequeue( qf[i], &a, &b, &c, &d, &e, &f, &g ) == SUCCESS )
		{
			// Process Request
			switch ( a )
			{
				case ADD_6_NUMS:
					b = add_6_nums( b, c, d, e, f, g );
					c = add_6_nums( b, c, d, e, f, g );
					break;

				case NULL_INVOCATION:
					null_invocation();
					null_invocation();
					break;

				case HALT:
					halt = 1;
					break;
			}

			node_t* node = (node_t*) g;
			node->a = a;
			node->b = b;
			node->c = c;
			node->d = d;
			node->e = e;
			node->f = f;
			node->next = NULL;
			while ( enqueue( qb[i], node ) != SUCCESS )
				fipc_test_pause();
		}

		++i; if ( i >= producer_count ) i = 0;
	}

	// End test
	fipc_test_mfence();
	pr_err( "Consumer %llu finished.\n", rank );
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI( completed_consumers );
	return 0;
}


int controller ( void* data )
{
	uint64_t i;
	uint64_t j;

	// Queue Allocation
	queue_t* queues_forw = (queue_t*) vmalloc( producer_count*consumer_count*sizeof(queue_t) );
	queue_t* queues_back = (queue_t*) vmalloc( producer_count*consumer_count*sizeof(queue_t) );

	for ( i = 0; i < producer_count*consumer_count; ++i )
	{
		init_queue ( &queues_forw[i] );
		init_queue ( &queues_back[i] );
	}

	prod_queues_forw = (queue_t***) vmalloc( producer_count*sizeof(queue_t**) );
	cons_queues_forw = (queue_t***) vmalloc( consumer_count*sizeof(queue_t**) );

	prod_queues_back = (queue_t***) vmalloc( consumer_count*sizeof(queue_t**) );
	cons_queues_back = (queue_t***) vmalloc( consumer_count*sizeof(queue_t**) );

	for ( i = 0; i < producer_count; ++i )
		prod_queues_forw[i] = (queue_t**) vmalloc( consumer_count*sizeof(queue_t*) );

	for ( i = 0; i < consumer_count; ++i )
		cons_queues_forw[i] = (queue_t**) vmalloc( producer_count*sizeof(queue_t*) );

	for ( i = 0; i < producer_count; ++i )
		prod_queues_back[i] = (queue_t**) vmalloc( consumer_count*sizeof(queue_t*) );

	for ( i = 0; i < consumer_count; ++i )
		cons_queues_back[i] = (queue_t**) vmalloc( producer_count*sizeof(queue_t*) );

	// Queue Linking
	for ( i = 0; i < producer_count; ++i )
	{
		for ( j = 0; j < consumer_count; ++j )
		{
			prod_queues_forw[i][j] = &queues_forw[i*producer_count + j];
			cons_queues_forw[j][i] = &queues_forw[i*producer_count + j];
		}
	}

	for ( i = 0; i < producer_count; ++i )
	{
		for ( j = 0; j < consumer_count; ++j )
		{
			prod_queues_back[i][j] = &queues_back[i*producer_count + j];
			cons_queues_back[j][i] = &queues_back[i*producer_count + j];
		}
	}

	// Node Table Allocation
	node_tables = (node_t**) vmalloc( producer_count*sizeof(node_t*) );

	for ( i = 0; i < producer_count; ++i )
		node_tables[i] = (node_t*) vmalloc( transactions*sizeof(node_t) );

	node_t* haltMsg = (node_t*) vmalloc( consumer_count*sizeof(node_t) );

	fipc_test_mfence();

	// Test Data Allocation

	test_results = (uint64_t*) vmalloc( producer_count*sizeof(uint64_t) );

	// Thread Allocation
	kthread_t** cons_threads = (kthread_t**) vmalloc( consumer_count*sizeof(kthread_t*) );
	kthread_t** prod_threads = NULL;

	if ( producer_count >= 2 )
		prod_threads = (kthread_t**) vmalloc( (producer_count-1)*sizeof(kthread_t*) );

	uint64_t* p_rank = (uint64_t*) vmalloc( producer_count*sizeof(uint64_t) );
	uint64_t* c_rank = (uint64_t*) vmalloc( consumer_count*sizeof(uint64_t) );

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
	p_rank[producer_count-1] = producer_count-1;
	producer( &p_rank[producer_count-1] );

	// Wait for producers to complete
	while ( completed_producers < producer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Tell consumers to halt
	for ( i = 0; i < consumer_count; ++i )
	{
		haltMsg[i].a = HALT;

		enqueue( prod_queues_forw[producer_count-1][i], &haltMsg[i] );
	}

	// Wait for consumers to complete
	while ( completed_consumers < consumer_count )
		fipc_test_pause();

	fipc_test_mfence();

	uint64_t sum = 0;
	for ( i = 0; i < producer_count; ++i )
	{
		sum += test_results[i];
	}

	pr_err("Latency: %llu cycles per message", sum / producer_count);

	// Clean up
	vfree( c_rank );
	vfree( p_rank );
	vfree( test_results );

	for ( i = 0; i < consumer_count; ++i )
		fipc_test_thread_free_thread( cons_threads[i] );

	for ( i = 0; i < (producer_count-1); ++i )
		fipc_test_thread_free_thread( prod_threads[i] );

	vfree( cons_threads );

	if ( prod_threads != NULL )
		vfree( prod_threads );

	vfree( haltMsg );

	for ( i = 0; i < producer_count; ++i )
		vfree( node_tables[i] );

	vfree( node_tables );

	for ( i = 0; i < consumer_count; ++i )
		vfree( cons_queues_forw[i] );

	for ( i = 0; i < producer_count; ++i )
		vfree( prod_queues_forw[i] );

	for ( i = 0; i < consumer_count; ++i )
		vfree( cons_queues_back[i] );

	for ( i = 0; i < producer_count; ++i )
		vfree( prod_queues_back[i] );

	vfree( cons_queues_forw );
	vfree( prod_queues_forw );

	vfree( cons_queues_back );
	vfree( prod_queues_back );

	for ( i = 0; i < producer_count*consumer_count; ++i )
		free_queue( &queues_forw[i] );

	for ( i = 0; i < producer_count*consumer_count; ++i )
		free_queue( &queues_back[i] );

	vfree( queues_forw );
	vfree( queues_back );

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
