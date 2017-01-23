/**
 *
 */
 
#ifndef LIBFIPC_TEST_LIBRARY_LOCK
#define LIBFIPC_TEST_LIBRARY_LOCK

#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>

#define NUM_CORES sysconf(_SC_NPROCESSORS_ONLN)
#define PAGE_SIZE sysconf(_SC_PAGESIZE)

#define PAGES_NEEDED(x) \
				(1UL << ((x) < PAGE_SIZE ? 0 : (x) - PAGE_SIZE))

#define STOPWATCH_TEST_REPITIONS 10000

typedef struct fipc_ring_channel Header;
typedef struct fipc_message Message;

/**
 * This inline helper function pins the specified process to the specified core. 
 */
static inline
int pin_process_to_CPU ( pid_t pid, size_t core )
{
	if ( core >= NUM_CORES )
		return EINVAL;
	
	cpu_set_t cpu_mask;
	CPU_ZERO( &cpu_mask );
	CPU_SET( core, &cpu_mask );
	
	return sched_setaffinity( pid, sizeof(cpu_set_t), &cpu_mask );
}

/**
 * This inline helper function pins the specified thread to the specified core. 
 */
static inline
int pin_thread_to_CPU ( pthread_t thread, size_t core )
{
	if ( core >= NUM_CORES )
		return EINVAL;
	
	cpu_set_t cpu_mask;
	CPU_ZERO( &cpu_mask );
	CPU_SET( core, &cpu_mask );
	
	return pthread_setaffinity_np( thread,
								   sizeof(cpu_set_t),
								   &cpu_mask );
}

/**
 * This inline helper function pins the currently executing thread to the
 * specified core. 
 */
static inline
int pin_this_thread_to_CPU ( size_t core )
{
	return pin_thread_to_CPU( pthread_self(), core );
}

/**
 * This inline helper function pins the currently executing process to the
 * specified core. 
 */
static inline
int pin_this_process_to_CPU ( size_t core )
{
	return pin_process_to_CPU( 0, core );
}

/**
 * This inline helper function gives the currently executing thread control
 * over the core.
 */
static inline
void take_control_of_CPU ( void )
{
	// Disable Interrupts
}

/**
 * This inline helper function releases control of the core.
 */
static inline
void release_control_of_CPU ( void )
{
	// Restore Interrupts
}

/**
 * Thread spawn function that passes the channel to the newly created thread and pins the thread to cpu_pin.
 */
static inline
pthread_t* test_fipc_spawn_thread_with_channel ( Header* channel, void* (*threadfn) (void* data), int cpu_pin )
{
	pthread_t* thread = malloc( sizeof(pthread_t) );
//	pthread_attr_t tattr;
	struct sched_param param;

/*	if ( pthread_attr_init ( &tattr ) )
	{
		fprintf(stderr, "%s\n", "Error while initializing thread attributes.");
		return NULL;
	}

	if ( pthread_attr_getschedparam ( &tattr, &param ) )
	{
		fprintf(stderr, "%s\n", "Error while polling thread attributes.");
		return NULL;
	}

	param.sched_priority = sched_get_priority_max(SCHED_RR);
	//printf("%d, %d, %d\n", sched_getscheduler(0), SCHED_OTHER, sched_get_priority_max(SCHED_OTHER));

	if ( pthread_attr_setschedparam ( &tattr, &param ) )
	{
		fprintf(stderr, "%s\n", "Error while setting thread attributes.");
		return NULL;
	}

	if ( pthread_create( thread, &tattr, threadfn, channel ) )
	{
		fprintf(stderr, "%s\n", "Error while creating thread");
		return NULL;
	}*/

	if ( pthread_create( thread, NULL, threadfn, channel ) )
	{
		fprintf(stderr, "%s\n", "Error while creating thread");
		return NULL;
	}

	param.sched_priority = sched_get_priority_max(SCHED_RR);

	if ( pthread_setschedparam( *thread, SCHED_RR, &param ) )
	{
		fprintf(stderr, "%s\n", "Error while setting thread priority");
		return NULL;
	}

	if ( pin_thread_to_CPU( *thread, cpu_pin ) )
	{
		fprintf(stderr, "%s%d\n", "Error while pinning thread to CPU: ", cpu_pin);
		return NULL;
	}
	
	return thread;
}

/**
 * TODO: Write Description
 */
static inline
void test_fipc_release_thread ( pthread_t* thread )
{
	pthread_cancel( *thread );
}

/**
 * TODO: Write Description
 */
static inline
int test_fipc_wait_for_thread ( pthread_t *thread )
{
	return pthread_join( *thread, NULL );
}

/**
 * TODO: write description
 */
static inline
int test_fipc_create_channel ( size_t bufferOrder, Header** h1, Header** h2 )
{
	int		errorCode = 0;
	void*	buffer1   = NULL;
	void*	buffer2   = NULL;
	Header*	tempH1    = NULL;
	Header*	tempH2    = NULL;
	
	// Allocate Buffer Pages
	buffer1 = aligned_alloc( PAGE_SIZE, PAGES_NEEDED(bufferOrder)*PAGE_SIZE );
	buffer2 = aligned_alloc( PAGE_SIZE, PAGES_NEEDED(bufferOrder)*PAGE_SIZE );
	
	if ( buffer1 == NULL || buffer2 == NULL )
	{
		free ( buffer1 );
		free ( buffer2 );
		return -ENOMEM;
	}
	
	// Initialize Buffers
	errorCode = fipc_prep_buffers( bufferOrder, buffer1, buffer2 );
	
	if ( errorCode )
	{
		free ( buffer1 );
		free ( buffer2 );
		return errorCode;
	}
	
	// Allocate Headers
	tempH1 = (Header*) malloc( sizeof(*tempH1) );
	tempH2 = (Header*) malloc( sizeof(*tempH2) );
	
	if ( tempH1 == NULL || tempH2 == NULL )
	{
		free ( buffer1 );
		free ( buffer2 );
		free ( tempH1 );
		free ( tempH2 );
		return -ENOMEM;
	}
	
	// Initialize Headers
	errorCode = fipc_ring_channel_init( tempH1, bufferOrder, buffer1, buffer2 );
	if ( errorCode )
	{
		free ( buffer1 );
		free ( buffer2 );
		free ( tempH1 );
		free ( tempH2 );
		return errorCode;
	}

	errorCode = fipc_ring_channel_init( tempH2, bufferOrder, buffer2, buffer1 );
	if ( errorCode )
	{
		free ( buffer1 );
		free ( buffer2 );
		free ( tempH1 );
		free ( tempH2 );
		return errorCode;
	}
	
	*h1 = tempH1;
	*h2 = tempH2;

	return errorCode;
}

/**
 * TODO: write description
 */
static inline
void test_fipc_free_channel ( size_t bufferOrder, Header* h1, Header* h2 )
{
	// Free Buffers
	free ( h1->tx.buffer );
	free ( h2->tx.buffer );
	
	// Free Headers
	free( h1 );
	free( h2 );
}

/**
 * TODO: write description
 */
static inline
int test_fipc_blocking_recv_start ( Header* channel, Message** out )
{
	int ret;
	
	while ( 1 )
	{
		// Poll until we get a message or error
		ret = fipc_recv_msg_start(channel, out);

		if ( !ret || ret != -EWOULDBLOCK )
		{
			return ret;
		}

		pthread_yield(); // cpu_relax();
	}

	return 0;
}

/**
 * TODO: write description
 */
static inline
int test_fipc_blocking_send_start ( Header* channel, Message** out )
{
	int ret;

	while ( 1 )
	{
		// Poll until we get a free slot or error
		ret = fipc_send_msg_start(channel, out);

		if ( !ret || ret != -EWOULDBLOCK )
		{
			return ret;
		}

		pthread_yield(); // cpu_relax();
	}

	return 0;
}

static inline
uint64_t test_fipc_start_stopwatch ( void )
{
	uint64_t stamp;
	
	/*
	 * Assumes x86
	 *
	 * rdtsc returns current cycle counter on cpu; 
	 * low 32 bits in %rax, high 32 bits in %rdx.
	 *
	 * Note: We use rdtsc to start the stopwatch because it won't
	 * wait for prior instructions to complete (that we don't care
	 * about). It is not exact - meaning that instructions after
	 * it in program order may start executing before the read
	 * is completed (so we may slightly underestimate the time to
	 * execute the intervening instructions). But also note that
	 * the two subsequent move instructions are also counted against
	 * us (overestimate).
	 */
	 
	asm volatile
	(
		"rdtsc\n\t"
		"shl $32, %%rdx\n\t"
		"or %%rdx, %%rax\n\t" 
		: "=a" (stamp)
		:
		: "rdx"
	);
	
	return stamp;
}

static inline
uint64_t test_fipc_stop_stopwatch ( void )
{
	uint64_t stamp;
	
	/*
	 * Assumes x86
	 *
	 * Unlike start_stopwatch, we want to wait until all prior
	 * instructions are done, so we use rdtscp. (We don't care
	 * about the tsc aux value.)
	 */
	 
	asm volatile
	(
		"rdtscp\n\t"
		"shl $32, %%rdx\n\t"
		"or %%rdx, %%rax\n\t" 
		: "=a" (stamp)
		:
		: "rdx", "rcx"
	);
	
	return stamp;
}

/**
 * This helper function returns average cost of using the stopwatch.
 */
static inline
uint64_t test_Average_Stopwatch_Delay ( void )
{
	uint64_t delayCostAcc = 0;
    int i; 
	
	for ( i = 0; i < STOPWATCH_TEST_REPITIONS; ++i )
	{
		uint64_t start = test_fipc_start_stopwatch();
		uint64_t end   = test_fipc_stop_stopwatch();
		delayCostAcc  += end - start;
	}
	
	return delayCostAcc / STOPWATCH_TEST_REPITIONS;
}
#endif

/**
 * An integer log base 2 helper function.
 */
static inline
unsigned int ilog2 (unsigned int val)
{
    if (val == 0) return UINT_MAX;
    if (val == 1) return 0;
    unsigned int ret = 0;
    while (val > 1)
    {
        val >>= 1;
        ret++;
    }
    return ret;
}
