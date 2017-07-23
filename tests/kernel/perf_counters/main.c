/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;
	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	for ( i = 0; i < BATCHED_ORDER; ++i )
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
	for ( i = 0; i < BATCHED_ORDER; ++i )
	{
		fipc_test_blocking_recv_start( chan, &request );
		fipc_recv_msg_end( chan, request );
	}

	for ( i = 0; i < BATCHED_ORDER; ++i )
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
	register int32_t* CACHE_ALIGNED times = vmalloc( TRANSACTIONS * sizeof( int32_t ) );

	e1 = kmalloc( sizeof( evt_sel_t ), GFP_KERNEL );
	e2 = kmalloc( sizeof( evt_sel_t ), GFP_KERNEL );
	e3 = kmalloc( sizeof( evt_sel_t ), GFP_KERNEL );
	e4 = kmalloc( sizeof( evt_sel_t ), GFP_KERNEL );

	create_event(0x24, 0x01, OS_MODE, e1);
	create_event(0x24, 0x02, OS_MODE, e2);
	create_event(0x24, 0x04, OS_MODE, e3);
	create_event(0x24, 0x08, OS_MODE, e4);

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	write_evtsel(e1, 0);
	write_evtsel(e2, 1);
	write_evtsel(e3, 2);
	write_evtsel(e4, 3);

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		request( chan );

		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
	}

	reset_evtsel(3);
	reset_evtsel(2);
	reset_evtsel(1);
	reset_evtsel(0);

	read_pmc(&e1->reg, 0);
	read_pmc(&e2->reg, 1);
	read_pmc(&e3->reg, 2);
	read_pmc(&e4->reg, 3);

	reset_pmc(0);
	reset_pmc(1);
	reset_pmc(2);
	reset_pmc(3);

	pr_err("%llu    %llu    %llu    %llu", e1->reg, e2->reg, e3->reg, e4->reg);

	// End test
	fipc_test_thread_release_control_of_CPU();
	fipc_test_stat_get_and_print_stats( times, TRANSACTIONS );
	vfree( times );
	kfree( e1 );
	kfree( e2 );
	kfree( e3 );
	kfree( e4 );
	complete( &requester_comp );
	return 0;
}

int responder ( void* data )
{
	header_t* chan = (header_t*) data;
	
	register uint64_t CACHE_ALIGNED transaction_id;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond( chan );
	}

	// End test
	fipc_test_thread_release_control_of_CPU();
	complete( &responder_comp );
	return 0;
}


int main ( void )
{
	init_completion( &requester_comp );
	init_completion( &responder_comp );

	header_t*  requester_header = NULL;
	header_t*  responder_header = NULL;
	kthread_t* requester_thread = NULL;
	kthread_t* responder_thread = NULL;

	fipc_init();
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header, &responder_header );

	if ( requester_header == NULL || responder_header == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}

	// Create Threads
	requester_thread = fipc_test_thread_spawn_on_CPU ( requester, requester_header, REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	responder_thread = fipc_test_thread_spawn_on_CPU ( responder, responder_header, RESPONDER_CPU );
	if ( responder_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// Start threads
	wake_up_process( requester_thread );
	wake_up_process( responder_thread );
	
	// Wait for thread completion
	wait_for_completion( &requester_comp );
	wait_for_completion( &responder_comp );
	
	// Clean up
	fipc_test_thread_free_thread( requester_thread );
	fipc_test_thread_free_thread( responder_thread );
	fipc_test_free_channel( CHANNEL_ORDER, requester_header, responder_header );
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
