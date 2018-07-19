/**
* @File     : main.c
* @Author   : Abdullah Younis
* @Author   : Minjun Cha
* @Author   : Jeonghoon Lee
*/

#include "test.h"

int __attribute__ ((noinline)) null_invocation(void)
{
	asm volatile ("");
	return 0;
}

void* producer(void* data)
{
	queue_t*   q = &queue;
	request_t* t = (request_t*)data;

	register uint64_t transaction_id;
	register uint64_t start;
	register uint64_t end;
	
	int temp = 0;

	// Touching data
	for (transaction_id = 0; transaction_id < transactions; transaction_id++)
	{
		t[transaction_id].data = 0;
		t[transaction_id].next = NULL;
	}

	// Begin test
	// fipc_test_thread_take_control_of_CPU();
	pthread_mutex_lock(&producer_mutex[mutex_num_producer]);
	temp = mutex_num_producer;
	// Wait for everyone to be ready

	fipc_test_FAI(mutex_num_producer);

	//fipc_test_mfence();
	start = RDTSC_START();

	for (transaction_id = 0; transaction_id < transactions; transaction_id++)
	{
		t[transaction_id].data = NULL_INVOCATION;

		enqueue(q, &t[transaction_id]);
	}

	end = RDTSCP();
	// End test
	printf("Producer Mutex Number is %d,  Producer completed in %llu and the average was %llu\n", temp, end-start, (end-start)/transactions);
	
	//fipc_test_thread_release_control_of_CPU();
	pthread_mutex_unlock(&producer_mutex[temp]);
	//pthread_mutex_unlock(&producer_mutex[mutex_num_producer]);

	return NULL;
}

void* consumer(void* data)
{
	queue_t* q = (queue_t*)data;

	uint64_t request;

	int halt = 0;
	int temp = 0;
	// Begin test
	// fipc_test_thread_take_control_of_CPU();
	pthread_mutex_lock(&consumer_mutex[mutex_num_consumer]);
	temp = mutex_num_consumer;

	// Wait for everyone to be ready
	fipc_test_FAI(mutex_num_consumer);

	//fipc_test_mfence();
	
	// Consume
	while (!halt)
	{
		// Receive and unmarshall request
		if (dequeue(q, &request) == SUCCESS)
		{
			// Process Request
			switch (request)
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
	// fipc_test_mfence();
	printf("Consumer Mutex Number is %d, CONSUMER FINISHING\n", temp);
	
	//fipc_test_thread_release_control_of_CPU();
	pthread_mutex_unlock(&consumer_mutex[temp]);
	//pthread_mutex_unlock(&consumer_mutex[mutex_num_consumer]);
	return NULL;
}


void* controller(void* data)
{
	int i;

	// Queue Init
	init_queue(&queue);

	// Node Table Allocation
	request_t** node_table = (request_t**) malloc( producer_count*sizeof(request_t*) );

	for ( i = 0; i < producer_count; ++i )
		node_table[i] = (request_t*) malloc( transactions*sizeof(request_t) );

	request_t* haltMsg = (request_t*) malloc( consumer_count*sizeof(request_t) );
	
	// Thread Allocation
	pthread_t** cons_threads = (pthread_t**) malloc( consumer_count*sizeof(pthread_t*) );
	pthread_t** prod_threads = (pthread_t**) malloc( producer_count*sizeof(pthread_t*) );

	for (i = 0; i < producer_count; ++i) 
	{
		pthread_mutex_init( &producer_mutex[i] , NULL );
		pthread_mutex_lock( &producer_mutex[i] );
	}

	for (i = 0; i < consumer_count; ++i)
	{
		pthread_mutex_init( &consumer_mutex[i], NULL );
		pthread_mutex_lock( &consumer_mutex[i] );
	}

	// Spawn Threads
	for ( i = 0; i < producer_count; ++i )
	{
		prod_threads[i] = fipc_test_thread_spawn_on_CPU( producer, node_table[i], producer_cpus[i]);
		
		if ( prod_threads[i] == NULL )
		{
			fprintf( stderr, "%s\n", "Error while creating thread" );
			return NULL;
		}
	}

	for ( i = 0; i < consumer_count; ++i )
	{
		cons_threads[i] = fipc_test_thread_spawn_on_CPU( consumer, &queue, consumer_cpus[i]);

		if ( cons_threads[i] == NULL )
		{
			fprintf( stderr, "%s\n", "Error while creating thread" );
			return NULL;
		}
	}

	//Begin test
	for ( i = 0; i < producer_count; ++i )
	{
		pthread_mutex_unlock( &producer_mutex[i] );
	}

	for ( i = 0; i < consumer_count; ++i )
	{
		pthread_mutex_unlock( &consumer_mutex[i] );
	}
	
	// Tell consumers to halt
	for (i = 0; i < consumer_count; ++i)
	{
		haltMsg[i].next = 0;
		haltMsg[i].data = HALT;

		enqueue( &queue, &haltMsg[i] );
	}

	//fipc_test_mfence();

	// Wait for thread completion
	for ( i = 0; i < producer_count; ++i )
		fipc_test_thread_wait_for_thread( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		fipc_test_thread_wait_for_thread( cons_threads[i] );

	// Clean up
	for ( i = 0; i < producer_count; ++i )
		fipc_test_thread_free_thread( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		fipc_test_thread_free_thread( cons_threads[i] );

//	for ( i = 0; i < producer_count; ++i )
//		free( node_table[i] );
/*

	if (prod_threads != NULL)
		free(prod_threads);

	free( cons_threads );
	free( node_table );
	free( haltMsg );
	free_queue( &queue );
*/
	// End Experiment
	//fipc_test_mfence();
	return 0;
}

int main ( void )
{
	controller(NULL);
	//pthread_exit( NULL );
	return 0;
}
