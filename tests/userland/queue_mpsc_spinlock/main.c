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

#endif

uint64_t prod_sum = 0;
uint64_t cons_sum = 0;
int* halt;

int match_cpus(uint32_t* producer_cpus, uint32_t* consumer_cpus)
{
	struct bitmask *cm;	
    int num_nodes;
    int n;
    unsigned long cpu_bmap;
    int numa_present;
    int ret = 0;
    int possible_cpus = numa_num_configured_cpus();

    struct numa_config *config;
    struct node *nodes;

    struct task_placement *all_cpus;

    // numa_available returns 0 if numa apis are available, else -1
    if ((ret = numa_present = numa_available())) {
            printf("Numa apis unavailable!\n");
            goto err_numa;
    }

    config = calloc(1, sizeof(struct numa_config));

    if (!config) {
            perror("calloc:");
            goto err_numa;
    }
/*
    printf("numa_available: %s\n", numa_present ? "false" : "true");
    printf("numa_max_possible_node: %d\n", numa_max_possible_node());
    printf("numa_num_possible_nodes: %d\n", numa_num_possible_nodes());
    printf("numa_max_node: %d\n", numa_max_node());
*/
    config->num_nodes = num_nodes = numa_num_configured_nodes();
    cm = numa_allocate_cpumask();

    config->nodes = nodes = calloc(num_nodes, sizeof(struct node));
/*
    printf("numa_num_configured_nodes: %d\n", numa_num_configured_nodes());
    printf("numa_num_configured_cpus: %d\n", numa_num_configured_cpus());
    printf("numa_num_possible_cpus: %d\n", numa_num_possible_cpus());
*/
    producer_cpus = calloc(possible_cpus, sizeof(uint32_t));
    consumer_cpus = calloc(possible_cpus, sizeof(uint32_t));

    for (n = 0; n < num_nodes; n++) {
            int num_cpus, cpus = 0;
            if ((ret = numa_node_to_cpus(n, cm))) {
                    fprintf(stderr, "bitmask is not long enough\n");
                    goto err_range;
            }

            nodes[n].cpu_bitmask = cpu_bmap = *(cm->maskp);
            nodes[n].num_cpus = num_cpus = __builtin_popcountl(cpu_bmap);
            nodes[n].cpu_list = calloc(sizeof(uint32_t), num_cpus);

            // extract all the cpus from the bitmask
            while (cpu_bmap) {
                    // cpu number starts from 0, ffs starts from 1.
                    unsigned long c = __builtin_ffsll(cpu_bmap) - 1;
                    cpu_bmap &= RESET_MASK(c);
                    nodes[n].cpu_list[cpus++] = c;
            }
    }

    int prod_id = 0;
    int cons_id = 0;

    for (n = 0; n < num_nodes; n++) {
            int cpu;
	int num_cpus = nodes[n].num_cpus;

//                printf("Node: %d cpu_bitmask: 0x%08lx | num_cpus: %d\n", n, nodes[n].cpu_bitmask,
//                                               nodes[n].num_cpus);
            
            for (cpu = 0; cpu < num_cpus / 2; cpu++) {
		int next = num_nodes * (num_cpus / 2);
		
		producer_cpus[prod_id] = nodes[n].cpu_list[cpu];
	        producer_cpus[prod_id + next] = nodes[n].cpu_list[cpu + (num_cpus / 2)];
		consumer_cpus[cons_id] = producer_cpus[prod_id + next];
		consumer_cpus[cons_id + next] = producer_cpus[prod_id];
		
		++prod_id;
		++cons_id;
            }
//              printf("\n");
            free(nodes[n].cpu_list);
    }
//	int i;
//	for (i = 0; i < 64; ++i){
//	        printf("producer_cpus[%d] :  %d, consumer_cpus[%d] : %d\n", i, producer_cpus[i], i, consumer_cpus[i]); 
//	}

        
    free(nodes);
	return 0;
err_range:
    numa_free_cpumask(cm);
err_numa:
	return ret;
}


int null_invocation ( void )
{
	asm volatile ("nop");
	return 0;
}
/*
#ifdef __KERNEL__
void *
#else
void *
#endif
producer ( void* data )
{
	queue_t** 	 q = full_queues;
	uint64_t rank = *(uint64_t*)data;
	node_t*   t = node_tables[rank];

	uint64_t transaction_id;
	uint64_t start;
	uint64_t end;
	uint64_t cons_id = 0;
	int i = 0; 

	// We have a fixed size object pool, we pick one object 
	// from that pool as transaction_id mod pool_size
	uint64_t obj_id_mask = ((1UL << mem_pool_order) - 1);	

	pr_err( "Producer %lu starting...\n", rank );

	// Wait for everyone to be ready
	fipc_test_FAI( ready_producers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();
	
	for ( transaction_id = 0; transaction_id < consumer_count * transactions; )
	{
		for(i = 0; i < batch_size; i++) 
		{
			node_t *node = &t[transaction_id & obj_id_mask]; 

			node->data = NULL_INVOCATION;
			
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

	end = RDTSCP();

	// End test
	pr_err( "Producer %lu finished, sending %lu messages (cycles per message %lu)\n", 
			rank,
			transaction_id, 
			(end - start) / transaction_id);

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
	queue_t** q = full_queues;

	uint64_t start;
	uint64_t end;
	uint64_t request;
	int i;	
	uint64_t transaction_id = 0;
	uint64_t rank = *(uint64_t*)data;		
	
	pr_err( "Consumer %llu starting\n", (unsigned long long)rank );

	// Begin test
	// fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );

	while ( !test_ready )
		fipc_test_pause();

	fipc_test_mfence();

	start = RDTSC_START();
	
	while(!halt[rank])
	{	
		for(i = 0; i < batch_size; i++) 
		{
			// Receive and unmarshall 
			if ( dequeue ( q[rank], &request ) != SUCCESS )
			{
				break;
			}
			else
			{
				// Process Request
				switch ( request )
				{
					case NULL_INVOCATION:
						null_invocation();
						break;
					case HALT:
						halt[rank] = 1;
						break;
				}
				
				transaction_id ++;			
				
			}

		}
	}

	end = RDTSCP();

	// End test
	fipc_test_mfence();
	pr_err( "Consumer %lu finished, receiving %lu messages (cycles per message %lu) (%s)\n", 
			rank,
			transaction_id, 
			(end - start) / transaction_id, 
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

	full_queues = (queue_t**) vmalloc( consumer_count*sizeof(queue_t*) );

	node_t* haltMsg = (node_t*) vmalloc( consumer_count*sizeof(node_t) );

	halt = (int*) vmalloc( consumer_count*sizeof(*halt) );
	
	for ( i = 0; i < consumer_count; ++i ) 
	{
		full_queues[i] = (queue_t*) vmalloc( sizeof(queue_t) );
		halt[i] = 0;
	}

	for ( i = 0; i < consumer_count; ++i )
		full_queues[i] = &queues[i];

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
		haltMsg[i].data = HALT;
		enqueue( full_queues[i], &haltMsg[i] );
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
	vfree( full_queues );

	// End Experiment
	fipc_test_mfence();
	test_finished = 1;
	return 0;
}
*/
#ifndef __KERNEL__
int main(int argc, char *argv[])
#else
int init_module(void)
#endif
{
	match_cpus(producer_cpus, consumer_cpus);
	fipc_test_mfence();

	printf("%d \n", producer_cpus[0]);

	return 0;
}
/*
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
*/
