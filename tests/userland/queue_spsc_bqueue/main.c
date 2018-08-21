/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */
#ifdef __KERNEL__
#include <linux/module.h>
#endif

#include "test.h"
#include <inttypes.h>

#ifndef __KERNEL__
#include <sched.h>

#define kthread_t pthread_t
#define vmalloc malloc
#define vfree free

#include <malloc.h>

#endif

uint64_t CACHE_ALIGNED prod_sum = 0;
uint64_t CACHE_ALIGNED cons_sum = 0;
int * halt;

// Queue Variables
static queue_t*** prod_queues = NULL;
static queue_t*** cons_queues = NULL;
static node_t**   node_tables = NULL;

#define END_MSG_MARKER		0xabcdef11u

int null_invocation ( void )
{
	asm volatile ("nop");
	return 0;
}

#ifdef __KERNEL__
void *
#else
void *
#endif
producer ( void* data )
{
	uint64_t transaction_id;
	uint64_t start;
	uint64_t end;
	uint64_t cons_id = 0;
	int i; 
	node_t *end_msg = calloc(sizeof(node_t), 1);

	// We have a fixed size object pool, we pick one object 
	// from that pool as transaction_id mod pool_size
	uint64_t obj_id_mask = ((1UL << mem_pool_order) - 1);
	uint64_t rank = *(uint64_t*)data;
	node_t*   t = node_tables[rank];
	queue_t **q = prod_queues[rank];

	pr_err( "Producer %lu starting...\n", rank );
	// Touching data
	//for ( transaction_id = 0; transaction_id < mem_pool_size; transaction_id++ )
	//{
	//	t[transaction_id].field = 0;
	//}

	// Begin test
	//fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI(ready_producers);

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();

	
	for ( transaction_id = 0; transaction_id < consumer_count * transactions; )
	{
		for(i = 0; i < batch_size; i++) {
			node_t *node = &t[transaction_id & obj_id_mask]; 

			node->field = transaction_id;
			//prod_sum += transaction_id; /* node->field; */
			pr_dbg("Sending, tid:%lu, mask%lu, mod:%lu\n", 
					transaction_id, obj_id_mask, transaction_id & obj_id_mask);

			//printf("[%d] Sending %d message to consumer: %d | q->head %d\n", rank, node->field, cons_id, q[cons_id]->head);
			if ( enqueue( q[cons_id], (data_t)node ) != SUCCESS )
			{
				//pr_err("Failed to enqueue tid:%llu\n", 
				//	(unsigned long long)transaction_id);
				break;
			}
			transaction_id ++;
		};

		++cons_id;

		if (cons_id >= consumer_count)
			cons_id = 0;
	}

	end = RDTSCP();
	end_msg->field = END_MSG_MARKER;
	printf("[%" PRId64 "] Sent %" PRId64 " messages\n", rank, transaction_id);
	for ( cons_id = 0; cons_id < consumer_count; cons_id++) {
		//fipc_test_mfence();
		pr_dbg("[%" PRId64 "] Sending %p:%lx message to consumer: %" PRId64 " | q->head %u\n",
					rank, end_msg, end_msg->field, cons_id, q[cons_id]->head);
		while (enqueue( q[cons_id], (data_t)end_msg) != SUCCESS) ;
	}
	// End test
	pr_err( "Producer %lu finished, sending %lu messages (cycles per message %lu) (prod_sum:%lu)\n", 
			rank,
			transaction_id, 
			(end - start) / transaction_id, prod_sum);

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
	uint64_t start;
	uint64_t end;
	uint64_t prod_id = 0;
	uint64_t transaction_id = 0;
	node_t   *node;
	node_t   **node_array; 
	int i, j;
	int stop = 0;
	int halts = 0;
	uint64_t rank = *(uint64_t*)data;
	queue_t** q = cons_queues[rank];

	node_array = (node_t **) memalign( FIPC_CACHE_LINE_SIZE, batch_size*sizeof(node_t*) );
	if(!node_array) {
		pr_err("Failed to allocate node_array\n");
		return 0;
	}

	pr_info( "Consumer %llu starting\n", (unsigned long long)rank );

	// Begin test
	// fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();

	//while(!halt[rank])
	while(!stop)
	{
		for(i = 0; i < batch_size; i++) {
			// Receive and unmarshall
			if (q[prod_id] != NULL) {
				if ( dequeue( q[prod_id], (data_t*)&node ) != SUCCESS ) {
					break;
				}
				//printf("[%d] Receiving %p:%lx from prod %lu | q->tail %d\n",
				//		rank, node, node->field, prod_id, q[prod_id]->tail);

				if (node->field == END_MSG_MARKER) {
					halts++;
					pr_dbg("[%" PRId64 "] Received HALT[%p:%lx] (%d) msg from %" PRId64 " | q->tail %u\n",
							rank, node, node->field, halts,
							prod_id, q[prod_id]->tail);
					if (halts == producer_count)
						stop = 1;
					q[prod_id] = NULL;
					break;
				}
	#ifdef TOUCH_VALUE
				cons_sum += node->field;
	#endif

	#ifdef PREFETCH_VALUE
				/* rw flag (0 -- read, 1 -- write),
				 * temporal locality (0..3, 0 -- no locality) */
				__builtin_prefetch (node, 0, 0);
				node_array[i] = node;
	#endif
				transaction_id ++;

			}
		}
#ifdef PREFETCH_VALUE
		for(j = 0; j < i; j++) {
			cons_sum += node_array[j]->field;
		}
#endif

		++prod_id; if ( prod_id >= producer_count ) prod_id = 0;
	}

	end = RDTSCP();

	// End test
	fipc_test_mfence();
	
	if(transaction_id) {
		pr_info( "Consumer %lu finished, receiving %lu messages (cycles per message %lu) (cons sum:%lu)\n", 
			rank,
			transaction_id, 
			(end - start) / transaction_id, 
			cons_sum);
	} else {
		pr_info( "Consumer %lu finished, receiving %lu messages (cons sum:%lu)\n", 
			rank,
			transaction_id, 
			cons_sum);

	}

	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI( completed_consumers );
	return 0;
}


void * controller ( void* data )
{
	uint64_t i;
	uint64_t j;

	mem_pool_size = 1 << mem_pool_order;

	printf("Producers: %d | consumers: %d\n", producer_count, consumer_count);
	// Queue Allocation
	pr_dbg("Allocating %lu bytes for queue_t of size %lu\n",
			producer_count*consumer_count*sizeof(queue_t), sizeof(queue_t));

	queue_t* queues = (queue_t*) memalign( FIPC_CACHE_LINE_SIZE, producer_count*consumer_count*sizeof(queue_t) );
	if(!queues) {
		pr_err("Failed to allocate queues\n");
		return 0;
	}

	for ( i = 0; i < producer_count*consumer_count; ++i )
		init_queue ( &queues[i] );

	prod_queues = (queue_t***) memalign( FIPC_CACHE_LINE_SIZE, producer_count*sizeof(queue_t**) );
	if(!prod_queues) {
		pr_err("Failed to allocate prod_queues\n");
		return 0;
	}
	cons_queues = (queue_t***) memalign( FIPC_CACHE_LINE_SIZE, consumer_count*sizeof(queue_t**) );
	if(!cons_queues) {
		pr_err("Failed to allocate cons_queues\n");
		return 0;
	}

	halt = (int*) vmalloc( consumer_count*sizeof(*halt) );
	if(!halt) {
		pr_err("Failed to allocate halt\n");
		return 0;
	}

	for ( i = 0; i < producer_count; ++i ) {
		prod_queues[i] = (queue_t**) memalign( FIPC_CACHE_LINE_SIZE, consumer_count*sizeof(queue_t*) );
		if(!prod_queues[i]) {
			pr_err("Failed to allocate prod_queues[%lu]\n", i);
			return 0;
		}
	};

	for ( i = 0; i < consumer_count; ++i ) {
		cons_queues[i] = (queue_t**) memalign( FIPC_CACHE_LINE_SIZE, producer_count*sizeof(queue_t*) );
		if(!cons_queues[i]) {
			pr_err("Failed to allocate cons_queues[%lu]\n", i);
			return 0;
		}

		halt[i] = 0;
	}

	// Queue Linking
	for ( i = 0; i < producer_count; ++i )
	{
		for ( j = 0; j < consumer_count; ++j )
		{
			pr_dbg("%s:associate %p with prod %lu, cons %lu\n",
					__func__, &queues[i*consumer_count + j], i, j);
			prod_queues[i][j] = &queues[i*consumer_count + j];
			cons_queues[j][i] = &queues[i*consumer_count + j];
		}
	}

	// Node Table Allocation
	node_tables = (node_t**) vmalloc( producer_count*sizeof(node_t*) );
	if(!node_tables) {
		pr_err("Failed to allocate node_tables\n");
		return 0;
	}


	for ( i = 0; i < producer_count; ++i ) {
		pr_dbg("Allocating %lu bytes for the pool of %lu objects (pool order:%lu)\n", 
			mem_pool_size*sizeof(node_t), mem_pool_size, mem_pool_order);

		node_tables[i] = (node_t*) memalign( FIPC_CACHE_LINE_SIZE, mem_pool_size*sizeof(node_t) );
		if(!node_tables[i]) {
			pr_err("Failed to allocate node_tables[%lu]\n", i);
			return NULL;
		}
		pr_dbg("Check nodes are mem aligned: (%p):%s\n", 
			node_tables[i],
			((uint64_t)node_tables[i] & (FIPC_CACHE_LINE_SIZE - 1)) ? "not aligned" : "aligned");

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
	if(!p_rank) {
		pr_err("Failed to allocate p_rank\n");
		return 0;
	}

	uint64_t* c_rank = (uint64_t*) vmalloc( consumer_count*sizeof(uint64_t) );
	if(!c_rank) {
		pr_err("Failed to allocate c_rank\n");
		return 0;
	}

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

	fipc_test_mfence();

	// This thread is also a producer
	p_rank[producer_count-1] = producer_count-1;
	producer( &p_rank[producer_count-1] );

	// Wait for producers to complete
	while ( completed_producers < producer_count )
		fipc_test_pause();

	fipc_test_mfence();

	// Tell consumers to halt
	//for ( i = 0; i < consumer_count; ++i ) {
//		halt[i] = 1;
//	}

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

	vfree( halt );

	for ( i = 0; i < producer_count; ++i )
		free( node_tables[i] );

	vfree( node_tables );

	for ( i = 0; i < consumer_count; ++i )
		free( cons_queues[i] );

	for ( i = 0; i < producer_count; ++i )
		free( prod_queues[i] );

	free( cons_queues );
	free( prod_queues );

	for ( i = 0; i < producer_count*consumer_count; ++i )
		free_queue( &queues[i] );

	free( queues );

	// End Experiment
	fipc_test_mfence();
	test_finished = 1;
	return 0;
}

#ifndef __KERNEL__
int main(int argc, char *argv[])
#else
int init_module(void)
#endif
{

	printf("<<<<<<<<<<<<<<<<<<<<<< Test Started\n");
#ifndef __KERNEL__
	if (argc == 2) {
		transactions = (uint64_t) strtoul(argv[1], NULL, 10);
		printf("Starting test with %lu transactions\n", transactions);

	} else if (argc == 3) {
		producer_count = strtoul(argv[1], NULL, 10);
		consumer_count = strtoul(argv[2], NULL, 10);
		printf("Starting test with prod count %d, cons count %d\n",
				producer_count, consumer_count);
	} else if (argc == 5) {
		producer_count = strtoul(argv[1], NULL, 10);
		consumer_count = strtoul(argv[2], NULL, 10);
		transactions = (uint64_t) strtoul(argv[3], NULL, 10);
		batch_size = (uint64_t) strtoul(argv[4], NULL, 10);

		printf("Starting test with prod count %d, cons count %d, %lu transactions, and batch size %lu\n", 
				producer_count, consumer_count, transactions, batch_size);
	}

#endif
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
	printf(">>>>>>>>>>>>>>>>>>>> Test finished\n");

	return 0;
}

#ifdef __KERNEL__
void cleanup_module(void)
{
	return;
}

MODULE_LICENSE("GPL");
#endif
