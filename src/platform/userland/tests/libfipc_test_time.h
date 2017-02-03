/**
 * @File     : libfipc_test_time.h
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This library contains timing helper functions for the several fipc tests.
 *
 * NOTE: This library assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_TIME_LIBRARY_LOCK
#define LIBFIPC_TEST_TIME_LIBRARY_LOCK

/**
 * This function returns a time stamp with no preceding fence instruction.
 */
static inline
uint64_t fipc_test_time_get_timestamp ( void )
{
	uint64_t stamp;
	 
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

/**
 * This function returns a time stamp with a preceding load fence instruction.
 */
static inline
uint64_t fipc_test_time_get_timestamp_lf ( void )
{
	uint64_t stamp;
	
	fipc_test_lfence();
	
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

/**
 * This function returns a time stamp with a preceding store fence instruction.
 */
static inline
uint64_t fipc_test_time_get_timestamp_sf ( void )
{
	uint64_t stamp;
	 
	fipc_test_sfence();
	
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

/**
 * This function returns a time stamp a preceding memory fence instruction.
 */
static inline
uint64_t fipc_test_time_get_timestamp_mf ( void )
{
	uint64_t stamp;
	
	fipc_test_mfence();
	
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

/**
 * This function returns the mean of the sample set.
 */
static inline
uint64_t fipc_test_time_get_mean ( uint64_t* sample_set, uint32_t sample_size )
{
	register float sum = 0;

	uint64_t i;

	for ( i = 0; i < sample_size; ++i )
	{
		sum += sample_set[i];
	}

	return sum / (float) sample_size;
}

#endif
