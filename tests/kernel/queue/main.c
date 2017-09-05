/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <linux/module.h>
#include "test.h"

static inline
void enqueue ( node_t** head, node_t** tail, uint64_t* response_data, uint64_t* error_data, uint64_t data )
{
	node_t* new_node = vmalloc( sizeof(node_t) );

	if ( new_node == NULL )
	{
		*error_data = NO_MEMORY;
		return;
	}
 
	new_node->data = data;
	new_node->next = *head;

	if ( *tail == NULL )
	{
		*head = *tail = new_node;
	}
	else
	{
		(*tail)->next = new_node;
		*tail         = new_node;
	}

	*response_data = 0;
	*error_data    = SUCCESS;
}

static inline
void dequeue( node_t** head, node_t** tail, uint64_t* response_data, uint64_t* error_data )
{
	if ( *head == NULL )
	{
		*error_data = EMPTY_COLLECTION;
		return;
	}

	node_t* temp = *head;
	*head        = (*head)->next;

	if ( *head == NULL )
		*tail = NULL;

	*response_data = temp->data;
	*error_data    = SUCCESS;

	vfree( temp );
}

static inline
void size( node_t** head, node_t** tail, uint64_t* response_data, uint64_t* error_data )
{
	*response_data = 0;
	node_t* iter   = *head;

	while ( iter != NULL )
	{
		(*response_data)++;
		iter = iter->next;
	}

	*error_data = SUCCESS;
}

static inline
void clear( node_t** head, node_t** tail, uint64_t* response_data, uint64_t* error_data )
{
	*response_data = 0;

	while ( *head != NULL )
	{
		node_t* temp = *head;
		*head        = (*head)->next;

		vfree( temp );
		(*response_data)++;
	}

	*tail       = NULL;
	*error_data = SUCCESS;
}

static inline
uint8_t hash ( uint64_t data )
{
	return data % slave_count;
}

int controller ( void* data )
{
	header_t** chans = (header_t**) data;

	message_t* request;
	message_t* response;

	uint64_t response_data;
	uint64_t error_data;

	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	start = RDTSC_START();

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		fipc_test_blocking_send_start( chans[ hash( transaction_id ) ], &request );
		request->flags   = ENQUEUE;
		request->regs[0] = transaction_id;
		fipc_send_msg_end( chans[ hash( transaction_id ) ], request );
	}

//	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
//	{
//		fipc_test_prefetchw( chans[ hash( transaction_id ) ]->rx.buffer[ transactions / slave_count ] );
//	}

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		fipc_test_blocking_recv_start( chans[ hash( transaction_id ) ], &response );
		response_data = response->regs[0];
		error_data    = response->regs[1];

		if ( error_data != SUCCESS )
			pr_err( "Error #: %llu", error_data );

		fipc_recv_msg_end( chans[ hash( transaction_id ) ], response );
	}

	end = RDTSCP();

	// Shut down slave threads
	uint64_t slave_index;

	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
	{
		fipc_test_blocking_send_start( chans[ slave_index ], &request );
		request->flags = HALT;
		fipc_send_msg_end( chans[ slave_index ], request );
	}

	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
	{
		fipc_test_blocking_recv_start( chans[ slave_index ], &response );
		fipc_recv_msg_end( chans[ slave_index ], response );
	}

	// End test
	pr_err ( "Average Cycles: %llu", ( end - start ) / transactions );
	fipc_test_thread_release_control_of_CPU();
	complete( &controller_comp );
	return 0;
}

int slave ( void* data )
{
	header_t*  chan = (header_t*) data;
	message_t* request;
	message_t* response;

	uint32_t request_type;
	uint64_t request_data;

	uint64_t response_data;
	uint64_t error_data;

	node_t* head = NULL;
	node_t* tail = NULL;

	int halt = 0;

	// Begin test
	fipc_test_thread_take_control_of_CPU();

	while ( !halt )
	{
		error_data = 0;
		
		// Receive and unmarshall request
		fipc_test_blocking_recv_start( chan, &request );
		request_type = request->flags;
		request_data = request->regs[0];
		fipc_recv_msg_end( chan, request );

		// Process Request
		switch ( request_type )
		{
			case ENQUEUE:
				enqueue( &head, &tail, &response_data, &error_data, request_data );
				break;

			case DEQUEUE:
				dequeue( &head, &tail, &response_data, &error_data );
				break;

			case SIZE:
				size( &head, &tail, &response_data, &error_data );
				break;

			case HALT:
				halt = 1;

			case CLEAR:
				clear( &head, &response_data, &error_data );
				break;

			default:
				error_data = INVALID_REQUEST_TYPE;
				break;
		}

		// Send Response
		fipc_test_blocking_send_start( chan, &response );
		response->regs[0] = response_data;
		response->regs[1] = error_data;
		fipc_send_msg_end( chan, response );
	}

	// End test
	fipc_test_thread_release_control_of_CPU();
	return 0;
}


int main ( void )
{
	init_completion( &controller_comp );

	int  slave_index;
	int* slave_thread_map = kmalloc ( slave_count*sizeof(int),  GFP_KERNEL );

	header_t**  slave_headers = kmalloc( slave_count*sizeof(header_t*),  GFP_KERNEL );
	header_t**  cntrl_headers = kmalloc( slave_count*sizeof(header_t*),  GFP_KERNEL );
	kthread_t** slave_threads = kmalloc( slave_count*sizeof(kthread_t*), GFP_KERNEL );
	kthread_t*  cntrl_thread  = NULL;

	fipc_init();

	// Setup Channels
	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
	{
		fipc_test_create_channel( CHANNEL_ORDER, &slave_headers[slave_index], &cntrl_headers[slave_index] );

		if ( slave_headers[slave_index] == NULL || cntrl_headers[slave_index] == NULL )
		{
			pr_err( "%s\n", "Error while creating channel" );
			return -1;
		}
	}

	// Populate Slave Thread Map
	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
	{
		if ( slave_index >= controller_cpu )
			slave_thread_map[slave_index] = slave_index + 1;
		else
			slave_thread_map[slave_index] = slave_index;
	}

	// Create Threads
	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
	{
		slave_threads[slave_index] = fipc_test_thread_spawn_on_CPU ( slave, slave_headers[slave_index], slave_thread_map[slave_index] );

		if ( slave_threads[slave_index] == NULL )
		{
			pr_err( "%s\n", "Error while creating thread" );
			return -1;
		}
	}

	cntrl_thread = fipc_test_thread_spawn_on_CPU ( controller, cntrl_headers, controller_cpu );

	if ( cntrl_thread == NULL )
	{
		pr_err( "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// Start threads
	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
		wake_up_process( slave_threads[slave_index] );

	wake_up_process( cntrl_thread );
	
	// Wait for thread completion
	wait_for_completion( &controller_comp );
	
	// Clean up
	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
		fipc_test_thread_free_thread( slave_threads[slave_index] );

	fipc_test_thread_free_thread( cntrl_thread );

	for ( slave_index = 0; slave_index < slave_count; slave_index++ )
		fipc_test_free_channel( CHANNEL_ORDER, slave_headers[slave_index], cntrl_headers[slave_index] );

	kfree( slave_thread_map );
	kfree( slave_headers );
	kfree( cntrl_headers );
	kfree( slave_threads );
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
