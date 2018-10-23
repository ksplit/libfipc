/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#ifdef __KERNEL__
#include <linux/module.h>
#endif

#include "test.h"

#ifndef __KERNEL__
#include <sched.h>

#define kthread_t pthread_t
#define vmalloc malloc
#define vfree free
#define pr_err printf

#define END_MSG_MARKER          0xabcdef11u

#endif

uint64_t prod_sum = 0;
uint64_t cons_sum = 0;
int* halt;

int null_invocation ( void )
{
	asm volatile ("");
	return 0;
}

#ifdef __KERNEL__
void *
#else
void *
#endif
producer ( void* data )
{
	uint64_t rank = *(uint64_t*)data;
	queue_t** q = cons_queues;
	node_t*   t = node_tables[rank];

	uint64_t cons_id = 0;
	uint64_t transaction_id;
	int i; 

	// We have a fixed size object pool, we pick one object 
	// from that pool as transaction_id mod pool_size
	uint64_t obj_id_mask = ((1UL << mem_pool_order) - 1);	

	pr_err( "Producer %lu (tid: %lu) starting\n", rank, pthread_self() );

	// Wait for everyone to be ready
	fipc_test_FAI( ready_producers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	prod_starts[rank] = RDTSC_START();
	
	for ( transaction_id = 0; transaction_id < transactions; )
	{
		for(i = 0; i < batch_size; i++) 
		{
			node_t *node = &t[transaction_id & obj_id_mask]; 
			node->data = transaction_id;
			
			if ( enqueue( q[cons_id], node ) != SUCCESS )
			{
				break;
			}
			transaction_id ++;
		}	
		++cons_id;

		if (cons_id >= consumer_count) 
			cons_id = 0;
	}
	
	prod_ends[rank] = RDTSCP();

	// End test
	pr_err( "Producer %lu finished, sending %lu messages (cycles per message %lu)\n", 
			rank,
			transaction_id, 
			(prod_ends[rank] - prod_starts[rank]) / transaction_id);

	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI(completed_producers);
	return 0;
}

#ifdef __KERNEL__
void *
#else
void *
#endif
consumer ( void* data )
{
	uint64_t rank = *(uint64_t*)data;		
	queue_t** q = cons_queues;
	
	uint64_t request;
	uint64_t transaction_id = 0;
	int i;
	
	pr_err( "Consumer %lu (tid: %lu) starting\n", rank, pthread_self() );

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	cons_starts[rank] = RDTSC_START();
	
	while(!halt[rank])
	{	
		for(i = 0; i < batch_size; i++) 
		{
			// Receive and unmarshall 
			if ( dequeue ( q[rank], &request ) != SUCCESS )
			{
				break;
			}
		
                        if ( request == END_MSG_MARKER )
                        {
                                halt[rank] = 1;
                                break;
                        }
                        transaction_id++;
		}
	}

	cons_ends[rank] = RDTSCP();

	// End test
	fipc_test_mfence();
	pr_err( "Consumer %lu finished, receiving %lu messages (cycles per message %lu) (%s)\n", 
			rank,
			transaction_id, 
			(cons_ends[rank] - cons_starts[rank]) / transaction_id, 
			prod_sum == cons_sum ? "PASSED" : "FAILED");

	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI( completed_consumers );
	return 0;
}


void * controller ( void* data )
{
	uint64_t i;
	mem_pool_size = 1 << mem_pool_order;

	// Queue Allocation
	queue_t* queues = (queue_t*) vmalloc( consumer_count*sizeof(queue_t) );
	
	// Queue Init
	for ( i = 0; i < consumer_count; ++i )
		init_queue ( &queues[i] );

	cons_queues = (queue_t**) vmalloc( consumer_count*sizeof(queue_t*) );

	node_t* haltMsg = (node_t*) vmalloc( consumer_count*sizeof(node_t) );
	halt = (int*) vmalloc( consumer_count*sizeof(*halt) );
	
	for ( i = 0; i < consumer_count; ++i ) 
	{
		cons_queues[i] = (queue_t*) vmalloc( sizeof(queue_t) );
		halt[i] = 0;
	}

	for ( i = 0; i < consumer_count; ++i )
		cons_queues[i] = &queues[i];

	// Node Table Allocation
	node_tables = (node_t**) vmalloc( producer_count*sizeof(node_t*) );

	for ( i = 0; i < producer_count; ++i ) {
		pr_err("Allocating %lu bytes for the pool of %lu objects (pool order:%lu)\n", 
			mem_pool_size*sizeof(node_t), mem_pool_size, mem_pool_order);
		node_tables[i] = (node_t*) vmalloc( mem_pool_size*sizeof(node_t) );
	}

	fipc_test_mfence();

	// Thread Allocation
	kthread_t** cons_threads = (kthread_t**) vmalloc( consumer_count*sizeof(kthread_t*) );
	kthread_t** prod_threads = NULL;

	// In case there is only one producer, the controller thread becomes 
	// that producer
	if ( producer_count > 1 )
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
			return NULL;
		}
	}

	for ( i = 0; i < consumer_count; ++i )
	{
		c_rank[i] = i;
		cons_threads[i] = fipc_test_thread_spawn_on_CPU ( consumer, &c_rank[i], consumer_cpus[i] );
		
		if ( cons_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return NULL;
		}
	}

#ifdef __KERNEL__
	// Start threads
	for ( i = 0; i < (producer_count-1); ++i )
		wake_up_process( prod_threads[i] );

	for ( i = 0; i < consumer_count; ++i )
		wake_up_process( cons_threads[i] );
#endif
	
	// Wait for threads to be ready for test
	while ( ready_consumers < consumer_count )
		fipc_test_pause();
	
	while ( ready_producers < (producer_count-1) )
		fipc_test_pause();

	fipc_test_mfence();

	// Begin Test
	test_ready = 1;
	global_start = RDTSCP();

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
		haltMsg[i].next = 0;
		haltMsg[i].data = END_MSG_MARKER;
		enqueue( cons_queues[i], &haltMsg[i] );
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
	
	if ( prod_threads != NULL )
		vfree( prod_threads );

	vfree( cons_threads );

	for ( i = 0; i < producer_count; ++i )
		vfree( node_tables[i] );

	vfree( node_tables );
	vfree( halt );
	
	for ( i = 0; i < consumer_count; ++i )
		free_queue( &queues[i] );
	
	vfree( queues );
	vfree( cons_queues );

	// End Experiment
	fipc_test_mfence();

	global_end = RDTSCP();	
	test_finished = 1;

	printf (">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	for(int i = 0; i < consumer_count; ++i)
		printf("Consumer %d, start %lu\t/ end %lu\n", i, cons_starts[i] - global_start, cons_ends[i] - global_start);

	for(int i = 0; i < producer_count; ++i)
		printf("Producer %d, start %lu\t/ end %lu\n", i, prod_starts[i] - global_start, prod_ends[i] - global_start);

	printf("Global start %lu / end %lu : %lu\n", global_start, global_end, global_end - global_start);
	
	return 0;
}

#ifndef __KERNEL__
int main(int argc, char *argv[])
#else
int init_module(void)
#endif
{

#ifndef __KERNEL__
	if (argc == 2) {
		transactions = (uint64_t) strtoul(argv[1], NULL, 10);
		printf("Starting test with %lu transactions\n", transactions);

	} 
	else if (argc == 3) {
		producer_count = strtoul(argv[1], NULL, 10);
		consumer_count = strtoul(argv[2], NULL, 10);
		printf("Starting test with prod count %d, cons count %d\n",
				producer_count, consumer_count);

	} 
	else if (argc == 4) {
		producer_count = strtoul(argv[1], NULL, 10);
		consumer_count = strtoul(argv[2], NULL, 10);
		policy = (uint8_t) strtoul(argv[3], NULL, 10);

		printf("Starting test with prod count %d, cons count %d, [%d] policy\n", 
				producer_count, consumer_count, policy);

	} 
	else if (argc == 5) {
		producer_count = strtoul(argv[1], NULL, 10);
		consumer_count = strtoul(argv[2], NULL, 10);
		transactions = (uint64_t) strtoul(argv[3], NULL, 10);
		batch_size = (uint64_t) strtoul(argv[4], NULL, 10);

		printf("Starting test with prod count %d, cons count %d, %lu transactions, and batch size %lu\n", 
				producer_count, consumer_count, transactions, batch_size);
	}

#endif
	match_cpus(&producer_cpus, &consumer_cpus, policy);
	fipc_test_mfence();

	kthread_t* controller_thread = fipc_test_thread_spawn_on_CPU ( controller, NULL, producer_cpus[producer_count-1] );

	if ( controller_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
#ifdef __KERNEL__
	wake_up_process( controller_thread );
#endif

	fipc_test_thread_wait_for_thread(controller_thread); 

	fipc_test_mfence();
	fipc_test_thread_free_thread( controller_thread );
	pr_err("Test finished\n");

	return 0;
}


#ifdef __KERNEL__
void cleanup_module(void)
{
	return;
}

MODULE_LICENSE("GPL");
#endif

