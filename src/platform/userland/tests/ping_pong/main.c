#include "../libfipc_test.h"

#define REQUESTER_CPU 1
#define RESPONDER_CPU 3

// Global Variables = Test Shared Memory
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;
volatile cache_line_t reqt_buffer;
volatile cache_line_t resp_buffer;

void* requester ( void* data )
{
	uint64_t start;
	uint64_t end;
	
	// Get modified state of reqt_buffer
	reqt_buffer.regs[0] = 0;
	fipc_test_mfence();
	
	// Wait to begin test
	pthread_mutex_lock( &sender_mutex );

	start = fipc_test_get_timestamp();

	// Get modified state of reqt_buffer
	reqt_buffer.regs[0] = 1;
	fipc_test_mfence();

	// Load request cache line from requester, who has it in modified state
	while ( resp_buffer.regs[0] != 1 )
		fipc_test_pause();

	end = fipc_test_get_timestamp_mf();

	printf("\t%lu\n", end - start);

	pthread_exit( 0 );
	return NULL;
}

void* responder ( void* data )
{
	// Get modified state of resp_buffer
	resp_buffer.regs[0] = 0;
	fipc_test_mfence();
	
	// Wait to begin test
	pthread_mutex_lock( &receiver_mutex );
	pthread_mutex_unlock( &sender_mutex );

	// Load request cache line from requester, who has it in modified state
	while ( reqt_buffer.regs[0] == 0 )
		fipc_test_pause();

	// Get modified state of resp_buffer
	resp_buffer.regs[0] = 1;
	fipc_test_mfence();
	
	pthread_exit( 0 );
	return NULL;
}


int main ( void )
{
	volatile cache_line_t reqt_buffer;
	volatile cache_line_t resp_buffer;
	
	// Begin critical section
	pthread_mutex_init( &requester_mutex, NULL );
	pthread_mutex_init( &responder_mutex, NULL );
	
	pthread_mutex_lock( &requester_mutex );
	pthread_mutex_lock( &responder_mutex );
	
	// Create Threads
	pthread_t* requester_thread = fipc_test_thread_spawn_on_CPU ( requester,
																NULL,
																REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	pthread_t* responder_thread = fipc_test_thread_spawn_on_CPU ( responder,
																NULL,
																RESPONDER_CPU );
	if ( responder_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// End critical section = start threads
	// NOTE: We only unlock the responder lock, which in turn will unlock the
	//       the requester lock; this way we ensure correct states of each
	//       cache line prior to starting the test.
	pthread_mutex_unlock( &responder_mutex );
	
	// Wait for thread completion
	fipc_test_wait_for_thread( &requester_mutex );
	fipc_test_wait_for_thread( &responder_mutex );
	
	// Clean up
	fipc_test_thread_free_thread( &requester_mutex );
	fipc_test_thread_free_thread( &responder_mutex );
	pthread_exit( NULL );
	return 0;
}