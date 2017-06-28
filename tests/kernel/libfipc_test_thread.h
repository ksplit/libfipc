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

	struct cpumask cpu_mask;

	cpumask_clear   ( &cpu_mask );
	cpumask_set_cpu ( cpu_pin, &cpu_mask );

	return sched_setaffinity( pid, sizeof(cpu_set_t), &cpu_mask );
}

/**
 * This inline helper function pins the specified thread to the specified core.
 */
static inline
int fipc_test_thread_pin_thread_to_CPU ( struct task_struct* thread, size_t cpu_pin )
{
	if ( cpu_pin >= NUM_CORES )
		return -EINVAL;

	struct cpumask cpu_mask;

	cpumask_clear   ( &cpu_mask );
	cpumask_set_cpu ( cpu_pin, &cpu_mask );
	
	return set_cpus_allowed_ptr ( thread, &cpu_mask );
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
	struct task_struct* thread = kthread_create_on_cpu( threadfn, data, "libfipc.%d", cpu_pin );

	if ( IS_ERR(thread) )
	{
		#ifdef FIPC_TEST_DEBUG
			pr_err( "Error creating kernel thread\n" );
		#endif

		return NULL;
	}

	// Bump reference count, so even if thread dies before we have
	// a chance to wait on it, we won't crash
	get_task_struct(thread);

	if ( fipc_test_thread_pin_thread_to_CPU( thread, cpu_pin ) )
	{
		#ifdef FIPC_TEST_DEBUG
			pr_err( "Error pinning kernel thread to CPU:%d\n", cpu_pin );
		#endif

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
