/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This test ping pongs a cache line from one core to another core, and back
 * around.
 *
 * NOTE: This test assumes an x86 architecture.
 *
 * CITE: http://spcl.inf.ethz.ch/Publications/.pdf/ramos-hoefler-cc-modeling.pdf
 */

 #include "../libfipc_test.h"

#define REQUESTER_CPU 0
#define RESPONDER_CPU 2
#define TRANSACTIONS  1000

// Global Variables = Test Shared Memory
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;
volatile cache_line_t reqt_buffer = { .msg_status = 0} ;
volatile cache_line_t resp_buffer = { .msg_status = 0} ;


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

unsigned long __attribute__((aligned(64))) resp_sequence = 1; 
unsigned long __attribute__((aligned(64))) req_sequence = 1;

static inline void request_2_lines ( void )
{
	// Wait until message is available (GetS)
	// Send request (GetM)
	reqt_buffer.msg_status = req_sequence;

	// Wait until message is received (GetS)
	while ( likely(resp_buffer.msg_status != req_sequence) )
		fipc_test_pause();

	req_sequence ++;
}

static inline void respond_2_lines ( void )
{
	// Wait until message is received (GetS)
	while ( likely(reqt_buffer.msg_status != resp_sequence ))
		fipc_test_pause();

	// Send response (GetM)
	resp_buffer.msg_status = resp_sequence;

	resp_sequence ++;
}

static inline void request ( void )
{
	// Send request (GetM)
	reqt_buffer.msg_status = req_sequence;

	// Wait until message is received (GetS)
	while ( unlikely(reqt_buffer.msg_status != (req_sequence + 1)))
		fipc_test_pause();

	req_sequence += 2;
}

static inline void respond ( void )
{
	// Wait until message is received (GetS)
	while ( unlikely(reqt_buffer.msg_status != resp_sequence ))
		fipc_test_pause();

	// Send response (GetM)
	reqt_buffer.msg_status = resp_sequence + 1;

	resp_sequence += 2;
}
void* requester ( void* data )
{
	uint64_t __attribute__((aligned(64))) start;
	uint64_t __attribute__((aligned(64))) end;
	uint32_t __attribute__((aligned(64))) transaction_id;
	unsigned long long __attribute__((aligned(64))) sum = 0;
	
	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );
	start = RDTSC_START();
//	start = fipc_test_time_get_timestamp();

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		//start = RDTSC_START();
		//start = fipc_test_time_get_timestamp();
		request();
		//request_2_lines();
		//vend   = fipc_test_time_get_timestamp(); // A memory fence here costs ~200 cycles

		//end = RDTSCP();
		//sum += end - start;
       
		//printf("\t%lu\n", end - start);
	}
//	end   = fipc_test_time_get_timestamp(); // A memory fence here costs ~200 cycles

	end = RDTSCP();
	sum += end - start;

	printf("Ping pong (one cache line) Average Round Trip Cycles:%llu\n", sum / TRANSACTIONS );

        req_sequence = 1;
	sum = 0;

       	start = RDTSC_START();

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		//start = RDTSC_START();
		//start = fipc_test_time_get_timestamp();
		request_2_lines();
		//vend   = fipc_test_time_get_timestamp(); // A memory fence here costs ~200 cycles

		//end = RDTSCP();
		//sum += end - start;
       
		//printf("\t%lu\n", end - start);
	}

	end = RDTSCP();
	sum += end - start;

	printf("Ping pong (two cache lines) Average Round Trip Cycles:%llu\n", sum / TRANSACTIONS );



	pthread_mutex_unlock( &requester_mutex );
	pthread_exit( 0 );
	return NULL;
}

void* responder ( void* data )
{
	// Wait to begin test
	pthread_mutex_lock( &responder_mutex );


	uint32_t __attribute__((aligned(64))) transaction_id;
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond();
	}

        resp_sequence = 1; 

	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		respond_2_lines(); 
	}


	pthread_mutex_unlock( &responder_mutex );
	pthread_exit( 0 );
	return NULL;
}


int main ( void )
{
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
	pthread_mutex_unlock( &responder_mutex );
	pthread_mutex_unlock( &requester_mutex );
	
	// Wait for thread completion
	fipc_test_thread_wait_for_thread( requester_thread );
	fipc_test_thread_wait_for_thread( responder_thread );
	
	// Clean up
	fipc_test_thread_free_thread( requester_thread );
	fipc_test_thread_free_thread( responder_thread );
	pthread_exit( NULL );
	return 0;
}
