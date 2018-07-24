/**
* @File     : main.c
* @Author   : Abdullah Younis
* @Author   : Minjun Cha
* @Author   : Jeonghoon Lee
*/

#include "test.h"

int __attribute__((noinline)) null_invocation(void)
{
	asm volatile ("");
	return 0;
}

void* producer(void* data)
{
	uint64_t rank = *(uint64_t*)data;
	request_t*   t = node_tables[rank];
	queue_t*   q = &queue;

	register uint64_t transaction_id;
	register uint64_t start;
	register uint64_t end;
	printf("Producer %lu starting...\n", rank);


	// Touching data
	for (transaction_id = 0; transaction_id < transactions; transaction_id++)
	{*   t = node_tables[rank];
		t[transaction_id].data = 0;
		t[transaction_id].next = NULL;
	}
	// Begin test
	// fipc_test_thread_take_control_of_CPU();

	//pthread_mutex_lock(&producer_mutex[rank]);
	// Wait for everyone to be ready
	fipc_test_FAI(ready_producers);

	while (!test_ready)
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();


	for (transaction_id = 0; transaction_id < transactions; transaction_id++)
	{
		t[transaction_id].data = NULL_INVOCATION;

		enqueue(q, &t[transaction_id]);
	}

	end = RDTSCP();
	// End test
	printf("Producer Mutex Number is %d,  Producer completed in %llu and the average was %llu\n", temp, end - start, (end - start) / transactions);

	//pthread_mutex_unlock(&producer_mutex[rank]);

	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI(completed_producers);

	return NULL;
}

void* consumer(void* data)
{
	uint64_t rank = *(uint64_t*)data;

	queue_t* q = (queue_t*)queue;

	uint64_t request;

	printf("Consumer %llu starting\n", (unsigned long long)rank);

	int halt = 0;
	// Begin test
	// fipc_test_thread_take_control_of_CPU();
//	pthread_mutex_lock(&consumer_mutex[rank]);

// Wait for everyone to be ready
	fipc_test_FAI(ready_consumers);

	while (!test_ready)
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();

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
	fipc_test_mfence();
	printf("Consumer Mutex Number is %d, CONSUMER FINISHING\n", rank);
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI(completed_consumers);
	return 0;
//	pthread_mutex_unlock(&consumer_mutex[rank]);
	return NULL;
}


void* controller(void* data)
{
	int i;

	// Queue Init
	init_queue(&queue);

	// Node Table Allocation
	request_t** node_table = (request_t**)malloc(producer_count * sizeof(request_t*));

	for (i = 0; i < producer_count; ++i)
		node_table[i] = (request_t*)malloc(transactions * sizeof(request_t));

	request_t* haltMsg = (request_t*)malloc(consumer_count * sizeof(request_t));

	// Thread Allocation
	pthread_t** cons_threads = (pthread_t**)malloc(consumer_count * sizeof(pthread_t*));
	pthread_t** prod_threads = (pthread_t**)malloc(producer_count * sizeof(pthread_t*));
	
	/*
	for (i = 0; i < producer_count; ++i)
	{
		pthread_mutex_init(&producer_mutex[i], NULL);
		pthread_mutex_lock(&producer_mutex[i]);
	}

	for (i = 0; i < consumer_count; ++i)
	{
		pthread_mutex_init(&consumer_mutex[i], NULL);
		pthread_mutex_lock(&consumer_mutex[i]);
	}
	*/

	// Spawn Threads
	for (i = 0; i < producer_count; ++i)
	{
		prod_threads[i] = fipc_test_thread_spawn_on_CPU(producer, node_table[i], producer_cpus[i]);

		if (prod_threads[i] == NULL)
		{
			fprintf(stderr, "%s\n", "Error while creating thread");
			return NULL;
		}
	}

	for (i = 0; i < consumer_count; ++i)
	{
		cons_threads[i] = fipc_test_thread_spawn_on_CPU(consumer, &queue, consumer_cpus[i]);

		if (cons_threads[i] == NULL)
		{
			fprintf(stderr, "%s\n", "Error while creating thread");
			return NULL;
		}
	}

	//Begin test

	test_ready = 1;

	fipc_test_mfence();

	// This thread is also a producer
	producer(producer_count - 1);

	// Wait for producers to complete
	while (completed_producers < producer_count)
		fipc_test_pause();

	fipc_test_mfence();

	/*
	for (i = 0; i < producer_count; ++i)
	{
		pthread_mutex_unlock(&producer_mutex[i]);
	}

	for (i = 0; i < consumer_count; ++i)
	{
		pthread_mutex_unlock(&consumer_mutex[i]);
	}
	*/

	// Tell consumers to halt
	for (i = 0; i < consumer_count; ++i)
	{
		haltMsg[i].next = 0;
		haltMsg[i].data = HALT;

		enqueue(&queue, &haltMsg[i]);
	}

	fipc_test_mfence();

	// Wait for thread completion
	for (i = 0; i < producer_count; ++i)
		fipc_test_thread_wait_for_thread(prod_threads[i]);

	for (i = 0; i < consumer_count; ++i)
		fipc_test_thread_wait_for_thread(cons_threads[i]);

	// Clean up
	for (i = 0; i < (producer_count - 1); ++i)
		fipc_test_thread_free_thread(prod_threads[i]);

	for (i = 0; i < consumer_count; ++i)
		fipc_test_thread_free_thread(cons_threads[i]);

	for (i = 0; i < producer_count; ++i)
		vfree(node_table[i]);

	if (prod_threads != NULL)
		vfree(prod_threads);

	vfree(cons_threads);
	vfree(node_table);
	vfree(haltMsg);
	free_queue(&queue);

	// End Experiment
	fipc_test_mfence();
	test_finished = 1;
	return 0;

}

int main(void)
{
	controller(NULL);
	//pthread_exit( NULL );
	return 0;
}
