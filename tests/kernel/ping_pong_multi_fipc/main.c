/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void send_rank ( header_t* chan, uint64_t rank )
{
	message_t* rank_msg;

	fipc_test_blocking_send_start( chan, &rank_msg );
	rank_msg->regs[0] = rank;
	fipc_send_msg_end ( chan, rank_msg );
}

static inline
void recv_rank ( header_t* chan, uint64_t* rank )
{
	message_t* rank_msg;

	fipc_test_blocking_recv_start( chan, &rank_msg );
	*rank = rank_msg->regs[0];
	fipc_recv_msg_end ( chan, rank_msg );
}

static inline
void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}
}

static inline
void respond ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &request );
		fipc_recv_msg_end( chan, request );
	}

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_send_start( chan, &response );
		fipc_send_msg_end( chan, response );
	}
}

int requester ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t CACHE_ALIGNED correction = fipc_test_time_get_correction();
	register int32_t* CACHE_ALIGNED times = vmalloc( transactions * sizeof( int32_t ) );

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	// Wait for everyone to be ready
	fipc_test_FAI( ready_consumers );
	while ( !test_ready ) fipc_test_pause();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		start = RDTSC_START();

		request( chan );

		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
	}

	// End test
	fipc_test_thread_release_control_of_CPU();
	fipc_test_stat_get_and_print_stats( times, transactions );
	vfree( times );
	fipc_test_FAI( completed_requesters );
	return 0;
}

int responder ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED shifted_transactions = (transactions*requester_count) / responder_count;
	register uint64_t CACHE_ALIGNED rank = 0;

	recv_rank( chan, &rank );

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < shifted_transactions; transaction_id++ )
	{
		respond( chan );
	}

	if ( rank < (transactions*requester_count) % responder_count )
	{
		respond( chan );
	}

	// End test
	fipc_test_thread_release_control_of_CPU();
	fipc_test_FAI( completed_responders );
	return 0;
}


int main ( void )
{
	uint64_t i;
	header_t*  requester_header = NULL;
	header_t*  responder_header = NULL;

	fipc_init();
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header, &responder_header );

	if ( requester_header == NULL || responder_header == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}

	// Thread Allocation
	kthread_t** req_threads = kmalloc( requester_count*sizeof(kthread_t*), GFP_KERNEL );
	kthread_t** res_threads = kmalloc( responder_count*sizeof(kthread_t*), GFP_KERNEL );

	// Spawn Threads
	for ( i = 0; i < responder_count; ++i )
	{
		res_threads[i] = fipc_test_thread_spawn_on_CPU ( responder, responder_header, responder_cpus[i] );
		
		// responders need to know their rank
		send_rank( responder_header, i );

		if ( res_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}

	for ( i = 0; i < requester_count; ++i )
	{
		req_threads[i] = fipc_test_thread_spawn_on_CPU ( requester, requester_header, requester_cpus[i] );

		if ( req_threads[i] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}

	// Start responders
	for ( i = 0; i < responder_count; ++i )
		wake_up_process( res_threads[i] );

	// Start requesters
	for ( i = 0; i < requester_count; ++i )
		wake_up_process( req_threads[i] );

	// Wait for responders to be ready
	while ( ready_responders < responder_count )
		fipc_test_pause();

	// Wait for requesters to be ready
	while ( ready_requesters < requester_count )
		fipc_test_pause();

	asm volatile ( "CPUID" );

	// Begin Test
	test_ready = 1;

	// Wait for requesters to complete
	while ( completed_requesters < requester_count )
		fipc_test_pause();

	// Wait for responders to complete
	while ( completed_responders < responder_count )
		fipc_test_pause();
	
	// Clean up
	for ( i = 0; i < requester_count; ++i )
		fipc_test_thread_free_thread( req_threads[i] );

	for ( i = 0; i < responder_count; ++i )
		fipc_test_thread_free_thread( res_threads[i] );

	fipc_test_free_channel( CHANNEL_ORDER, requester_header, responder_header );
	kfree( res_threads );
	kfree( req_threads );
	fipc_fini();
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
