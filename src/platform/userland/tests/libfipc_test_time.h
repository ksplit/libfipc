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
uint64_t fipc_test_get_timestamp ( void )
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
uint64_t fipc_test_get_timestamp_lf ( void )
{
	uint64_t stamp;
	
	lfence();
	
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
uint64_t fipc_test_get_timestamp_sf ( void )
{
	uint64_t stamp;
	 
	sfence();
	
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
uint64_t fipc_test_get_timestamp_mf ( void )
{
	uint64_t stamp;
	
	mfence();
	
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

#endif