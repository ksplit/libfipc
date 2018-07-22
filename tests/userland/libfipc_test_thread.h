/**
 * @File     : libfipc_test_thread.h
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This library contains thread and process helper functions for the several
 * fipc tests.
 *
 * NOTE: This library assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_THREAD_LIBRARY_LOCK
#define LIBFIPC_TEST_THREAD_LIBRARY_LOCK

/**
 * This inline helper function pins the specified process to the specified core.
 */
static inline
int fipc_test_thread_pin_process_to_CPU ( pid_t pid, size_t cpu_pin )
{
	if ( cpu_pin >= NUM_CORES )
		return -EINVAL;

	cpu_set_t cpu_mask;
	CPU_ZERO( &cpu_mask );
	CPU_SET( cpu_pin, &cpu_mask );

	return sched_setaffinity( pid, sizeof(cpu_set_t), &cpu_mask );
}

/**
 * This inline helper function pins the specified thread to the specified core.
 */
static inline
int fipc_test_thread_pin_thread_to_CPU ( pthread_t thread, size_t cpu_pin )
{
	printf("Pinning thread to cpu:%zu\n", cpu_pin); 

	if ( cpu_pin >= NUM_CORES ) {
		printf("%s:Error: cpu:%zu id exeeds number of cores %lu\n", 
			__func__, cpu_pin, NUM_CORES); 

		return -EINVAL;
	}

	cpu_set_t cpu_mask;
	CPU_ZERO( &cpu_mask );
	CPU_SET( cpu_pin, &cpu_mask );

	return pthread_setaffinity_np( thread,
					   sizeof(cpu_set_t),
					   &cpu_mask );
}

/**
 * This inline helper function pins the currently executing thread to the
 * specified core.
 */
static inline
int fipc_test_thread_pin_this_thread_to_CPU ( size_t cpu_pin )
{
	return fipc_test_thread_pin_thread_to_CPU( pthread_self(), cpu_pin );
}

/**
 * This inline helper function pins the currently executing process to the
 * specified core.
 */
static inline
int fipc_test_thread_pin_this_process_to_CPU ( size_t cpu_pin )
{
	return fipc_test_thread_pin_process_to_CPU( 0, cpu_pin );
}

/**
 * This inline helper function gives the currently executing thread control
 * over the core.
 *
 * NOTE: This function is just a stub for possible future use.
 */
static inline
int fipc_test_thread_take_control_of_CPU ( void )
{
	// Disable Interrupts

	return 0;
}

/**
 * This inline helper function releases control of the core.
 *
 * NOTE: This function is just a stub for possible future use.
 */
static inline
int fipc_test_thread_release_control_of_CPU ( void )
{
	// Restore Interrupts

	return 0;
}

/**
 * This function (1) spawns a thread with the parameters specified,
 *               (2) pins that thread to cpu specified.
 */
static inline
pthread_t* fipc_test_thread_spawn_on_CPU ( void* (*threadfn)(void* data),
	void* data, size_t cpu_pin )
{
	pthread_t* thread = malloc( sizeof( pthread_t ) );

	if ( pthread_create( thread, NULL, threadfn, data ) )
	{
		#ifdef FIPC_TEST_DEBUG
			fprintf( stderr, "%s\n", "Error while creating thread" );
		#endif

		free( thread );
		return NULL;
	}


	if ( fipc_test_thread_pin_thread_to_CPU( *thread, cpu_pin ) )
	{
		#ifdef FIPC_TEST_DEBUG
			fprintf( stderr, "%s%d\n", "Error while pinning thread to CPU: ",
																	cpu_pin );
		#endif

		free( thread );
		return NULL;
	}

	return thread;
}

/**
 * This function deallocates a thread created with spawn_thread_on_CPU.
 */
static inline
int fipc_test_thread_free_thread ( pthread_t* thread )
{
	free ( thread );
	return 0;
}

/**
 * This function kills the thread specified.
 */
static inline
int fipc_test_thread_release_thread ( pthread_t* thread )
{
	return pthread_cancel( *thread );
}

/**
 * This function will wait for the specified thread to finish execution.
 */
static inline
int fipc_test_thread_wait_for_thread ( pthread_t* thread )
{
	return pthread_join( *thread, NULL );
}

/**
 * This function will wait for the specified thread to finish and return it's
 * exit code. (_ar = _and_return)
 */
static inline
int fipc_test_thread_wait_for_thread_ar ( pthread_t* thread, void* ret )
{
	return pthread_join( *thread, &ret );
}

#endif
