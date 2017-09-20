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
	queue_t* q = (queue_t*) data;

	register uint64_t transaction_id;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
		while( enqueue( q, NULL_INVOCATION ) );

	// End test
	fipc_test_thread_release_control_of_CPU();
	completed_producers++;
	return 0;
}

int consumer ( void* data )
{
	queue_t* q = (queue_t*) data;

	uint64_t request_type;

	int halt = 0;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	while ( !halt )
	{
		// Receive and unmarshall request
		while ( dequeue( q, &request_type ) );

		// Process Request
		switch ( request_type )
		{
			case NULL_INVOCATION:
				null_invocation();
				break;

			case HALT:
				halt = 1;
				break;
		}
	}

	// End test
	fipc_test_thread_release_control_of_CPU();
	completed_consumers++;
	return 0;
}


int main ( void )
{
	int i;

	// Queue Init
	init_queue ( &queue );

	// Thread Allocation
	kthread_t** prod_threads = kmalloc( producer_count*sizeof(kthread_t*), GFP_KERNEL );
	kthread_t** cons_threads = kmalloc( consumer_count*sizeof(kthread_t*), GFP_KERNEL );

	// Spawn Threads
	for ( i = 0; i < producer_count; ++i )
	{
		prod_threads[i] = fipc_test_thread_spawn_on_CPU ( producer, &queue, producer_cpus[i] );

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
	for ( i = 0; i < producer_count; ++i )
		wake_up_process( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		wake_up_process( cons_threads[i] );
	
	// Wait for producers to complete
	while ( completed_producers < producer_count )
		fipc_test_pause();

	// Tell consumers to halt
	for ( i = 0; i < consumer_count; ++i )
		while( enqueue( &queue, HALT ) );

	// Wait for consumers to complete
	while ( completed_consumers < consumer_count )
		fipc_test_pause();

	// Clean up
	for ( i = 0; i < producer_count; ++i )
		fipc_test_thread_free_thread( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		fipc_test_thread_free_thread( cons_threads[i] );

	free_queue( &queue );
	kfree( cons_threads );
	kfree( prod_threads );
	return 0;
}

int init_module(void)
{
	return main();
}

void cleanup_module(void)
{
	return;
}

MODULE_LICENSE("GPL");
