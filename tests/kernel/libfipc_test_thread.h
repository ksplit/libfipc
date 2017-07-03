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
 * This inline helper function pins the specified thread to the specified core.
 */
static inline
int fipc_test_thread_pin_thread_to_CPU ( kthread_t* thread, size_t cpu_pin )
{
	struct cpumask cpu_mask;

	if ( cpu_pin >= NUM_CORES )
		return -EINVAL;

	cpumask_clear   ( &cpu_mask );
	cpumask_set_cpu ( cpu_pin, &cpu_mask );
	
	return set_cpus_allowed_ptr ( thread, &cpu_mask );
}

/**
 * This inline helper function gives the currently executing thread control
 * over the core.
 */
static inline
int fipc_test_thread_take_control_of_CPU ( void )
{
	// Disable Interrupts
	local_irq_disable();
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
	local_irq_enable();
	return 0;
}

/**
 * This function (1) spawns a thread with the parameters specified,
 *               (2) pins that thread to cpu specified.
 */
static inline
kthread_t* fipc_test_thread_spawn_on_CPU ( int (*threadfn)(void* data),
											void* data, size_t cpu_pin )
{
	kthread_t* thread = kthread_create( threadfn, data, "libfipc.%lu", cpu_pin );

	if ( IS_ERR(thread) )
	{
		#ifdef FIPC_TEST_DEBUG
			pr_err( "Error creating kernel thread\n" );
		#endif

		return NULL;
	}

	// Bump reference count, so even if thread dies before we have
	// a chance to wait on it, we won't crash
	get_task_struct( thread );

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
int fipc_test_thread_free_thread ( kthread_t* thread )
{
	int ret;
	ret = kthread_stop( thread );
	put_task_struct( thread );
	kfree ( thread );
	return ret;
}

#endif
