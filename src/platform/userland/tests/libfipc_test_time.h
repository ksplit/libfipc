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

static unsigned long RDTSC_START(void)
{

        unsigned cycles_low, cycles_high;

        asm volatile ("CPUID\n\t"
                      "RDTSC\n\t"
                      "mov %%edx, %0\n\t"
                      "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
                      "%rax", "%rbx", "%rcx", "%rdx");
        return ((unsigned long) cycles_high << 32) | cycles_low;

}


static unsigned long RDTSCP(void)
{
        unsigned long tsc;
        __asm__ __volatile__(
        "rdtscp;"
        "shl $32, %%rdx;"
        "or %%rdx, %%rax"
        : "=a"(tsc)
        :
        : "%rcx", "%rdx");

        return tsc;
}


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
 * Quickselect: returns the index of the kth greatest value in the sample data
 *
 * CITE: http://www.cs.columbia.edu/~bert/code/bmatching/bdmatch/quickselect.c
 *
 * NOT WORKING
 */
uint64_t fipc_test_time_quickselect_r ( uint64_t* sample_set, uint64_t* indicies, uint64_t start, uint64_t end, uint64_t k )
{
	if ( start == end - 1 )
		return indicies[start];

	uint64_t pivot = start;

	uint64_t i;
	for ( i = start + 1; i < end; ++i )
	{
		if ( sample_set[i] > sample_set[pivot] )
		{
			uint64_t temp       = indicies[pivot];
			indicies[pivot]     = indicies[pivot + 1];
			indicies[pivot + 1] = temp;

			temp                  = sample_set[pivot];
			sample_set[pivot]     = sample_set[pivot + 1];
			sample_set[pivot + 1] = temp;

			if ( i > pivot + 1 )
			{
				temp            = indicies[pivot];
				indicies[pivot] = indicies[i];
				indicies[i]     = temp;

				temp              = sample_set[pivot];
				sample_set[pivot] = sample_set[i];
				sample_set[i]     = temp;
			}

			pivot++;
		}
	}

	if ( pivot == k )
		return indicies[pivot];
    else if ( pivot > k )
        return fipc_test_time_quickselect_r( sample_set, indicies, start, pivot, k );
    else
        return fipc_test_time_quickselect_r( sample_set, indicies, pivot+1, end, k );
}

static inline
uint64_t fipc_test_time_quickselect ( uint64_t* sample_set, uint64_t sample_size, uint64_t k )
{
	// Error checking
	if ( k >= sample_size || k < 0 )
		return -1;

	uint64_t* sample_copy = (uint64_t*) malloc ( sample_size * sizeof(uint64_t) );
	uint64_t* indicies    = (uint64_t*) malloc ( sample_size * sizeof(uint64_t) );

	uint64_t i;
	for ( i = 0; i < sample_size; ++i )
	{
		sample_copy[i] = sample_set[i];
		indicies[i]    = i;
	}

	uint64_t to_return = fipc_test_time_quickselect_r ( sample_copy, indicies, 0, sample_size, k );

	free ( sample_copy );
	free ( indicies );
	return to_return;
}

/**
 * This function returns the mean of the sample set.
 */
static inline
uint64_t fipc_test_time_get_mean ( uint64_t* sample_set, uint64_t sample_size )
{
	register float sum = 0;

	uint64_t i;

	for ( i = 0; i < sample_size; ++i )
	{
		sum += sample_set[i];
	}

	return sum / (float) sample_size;
}

/**
 * This function prints statistics of the sample set to stdin.
 */
int fipc_test_time_print_info ( uint64_t* sample_set, uint64_t sample_size )
{
	// Error Check
	if ( sample_set == NULL || sample_size == 0 )
		return -1;

	// Gather Info
	float sum   = 0;
	float sum25 = 0;
	float sum50 = 0;
	float sum75 = 0;
	float sum00 = 0;

	float ordered_sum25 = 0;
	float ordered_sum50 = 0;
	float ordered_sum75 = 0;
	float ordered_sum00 = 0;

	uint64_t i25 = sample_size/4;
	uint64_t i50 = sample_size/2;
	uint64_t i75 = 3*(sample_size/4);
	uint64_t i00 = sample_size;

	uint64_t x25 = fipc_test_time_quickselect( sample_set, sample_size, i25 );
	uint64_t x50 = fipc_test_time_quickselect( sample_set, sample_size, i50 );
	uint64_t x75 = fipc_test_time_quickselect( sample_set, sample_size, i75 );
	uint64_t x00 = fipc_test_time_quickselect( sample_set, sample_size, i00 );

	uint64_t min   = sample_set[0];
	uint64_t min25 = sample_set[i25 - 1];
	uint64_t min50 = sample_set[i50 - 1];
	uint64_t min75 = sample_set[i75 - 1];
	uint64_t min00 = sample_set[i00 - 1];

	uint64_t max   = sample_set[0];
	uint64_t max25 = sample_set[i25 - 1];
	uint64_t max50 = sample_set[i50 - 1];
	uint64_t max75 = sample_set[i75 - 1];
	uint64_t max00 = sample_set[i00 - 1];

	// One pass thru data
	uint64_t i = 0;
	for ( ;  i < i25; ++i )
	{
		sum   += sample_set[i];
		sum25 += sample_set[i];

		if ( sample_set[i] < sample_set[x25] )
		{
			ordered_sum25 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x50] )
		{
			ordered_sum50 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x75] )
		{
			ordered_sum75 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x00] )
		{
			ordered_sum00 += sample_set[i];
		}

		if ( sample_set[i] < min )
			min   = sample_set[i];
		if ( sample_set[i] < min25 )
			min25 = sample_set[i];

		if ( sample_set[i] > max )
			max   = sample_set[i];
		if ( sample_set[i] > max25 )
			max25 = sample_set[i];
	}
	for ( ; i < i50; ++i )
	{
		sum   += sample_set[i];
		sum50 += sample_set[i];

		if ( sample_set[i] < sample_set[x25] )
		{
			ordered_sum25 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x50] )
		{
			ordered_sum50 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x75] )
		{
			ordered_sum75 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x00] )
		{
			ordered_sum00 += sample_set[i];
		}
		
		if ( sample_set[i] < min )
			min   = sample_set[i];
		if ( sample_set[i] < min50 )
			min50 = sample_set[i];

		if ( sample_set[i] > max )
			max   = sample_set[i];
		if ( sample_set[i] > max50 )
			max50 = sample_set[i];
	}
	for ( ; i < i75; ++i )
	{
		sum   += sample_set[i];
		sum75 += sample_set[i];

		if ( sample_set[i] < sample_set[x25] )
		{
			ordered_sum25 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x50] )
		{
			ordered_sum50 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x75] )
		{
			ordered_sum75 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x00] )
		{
			ordered_sum00 += sample_set[i];
		}
		
		if ( sample_set[i] < min )
			min   = sample_set[i];
		if ( sample_set[i] < min75 )
			min75 = sample_set[i];

		if ( sample_set[i] > max )
			max   = sample_set[i];
		if ( sample_set[i] > max75 )
			max75 = sample_set[i];
	}
	for ( ; i < i00; ++i )
	{
		sum   += sample_set[i];
		sum00 += sample_set[i];

		if ( sample_set[i] < sample_set[x25] )
		{
			ordered_sum25 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x50] )
		{
			ordered_sum50 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x75] )
		{
			ordered_sum75 += sample_set[i];
		}
		else if ( sample_set[i] < sample_set[x00] )
		{
			ordered_sum00 += sample_set[i];
		}
		
		if ( sample_set[i] < min )
			min   = sample_set[i];
		if ( sample_set[i] < min00 )
			min00 = sample_set[i];

		if ( sample_set[i] > max )
			max   = sample_set[i];
		if ( sample_set[i] > max00 )
			max00 = sample_set[i];
	}

	// Calculate Statistics
	float mean   = sum   / (float) sample_size;
	float mean25 = sum25 / (float) ( i25 - 0 );
	float mean50 = sum50 / (float) ( i50 - i25 );
	float mean75 = sum75 / (float) ( i75 - i50 );
	float mean00 = sum00 / (float) ( i00 - i75 );

	float ordered_mean25 = ordered_sum25 / (float) ( i25 - 0 );
	float ordered_mean50 = ordered_sum50 / (float) ( i50 - i25 );
	float ordered_mean75 = ordered_sum75 / (float) ( i75 - i50 );
	float ordered_mean00 = ordered_sum00 / (float) ( i00 - i75 );

	// Print Statistics
	printf ( "Entire Data Statistics\n" );
	printf ( "\tAverage value: %f\n", mean );
	printf ( "\tMinimum value: %lu\n", min );
	printf ( "\tMaximum value: %lu\n", max );

	printf ( "\n" );

	printf ( "Split Data Statistics based on data order\n" );
	printf ( "\tAverage value (00%%-25%%): %f\n", mean25 ); 
	printf ( "\tMinimum value (00%%-25%%): %lu\n", min25 );
	printf ( "\tMaximum value (00%%-25%%): %lu\n\n", max25 );

	printf ( "\tAverage value (25%%-50%%): %f\n", mean50 ); 
	printf ( "\tMinimum value (25%%-50%%): %lu\n", min50 );
	printf ( "\tMaximum value (25%%-50%%): %lu\n\n", max50 );

	printf ( "\tAverage value (50%%-75%%): %f\n", mean75 ); 
	printf ( "\tMinimum value (50%%-75%%): %lu\n", min75 );
	printf ( "\tMaximum value (50%%-75%%): %lu\n\n", max75 );

	printf ( "\tAverage value (75%%-00%%): %f\n", mean00 );
	printf ( "\tMinimum value (75%%-00%%): %lu\n", min00 );
	printf ( "\tMaximum value (75%%-00%%): %lu\n\n", max00 );

	printf ( "\n" );

	printf ( "Split Data Statistics based on sorted order\n" );
	printf ( "\tAverage value (00%%-25%%): %f\n", ordered_mean25 ); 
	printf ( "\tAverage value (25%%-50%%): %f\n", ordered_mean50 ); 
	printf ( "\tAverage value (50%%-75%%): %f\n", ordered_mean75 ); 
	printf ( "\tAverage value (75%%-00%%): %f\n\n", ordered_mean00 );


	return 0;
}

#endif
