/**
 * @File     : libfipc_test_stat.h
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This library contains statistical helper functions for the several fipc tests.
 *
 * NOTE: This library assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_STAT_LIBRARY_LOCK
#define LIBFIPC_TEST_STAT_LIBRARY_LOCK

#define ABSOLUTE(x) ((x) > 0 ? (x) : -(x))
#define OUTLIER_THRESHOLD 3000

#include <math.h>

typedef struct fipc_test_stat_t
{
	uint64_t N;			// The size of the sample set
	float mean;			// The mean of the data
	float stdev;		// The standard deviation
	float abdev;		// The mean absolute deviation
	uint64_t min;		// The minimum value
	uint64_t max;		// The maximum value
	uint64_t outliers;	// The number of data points > OUTLIER_THRESHOLD
	float normMean;		// The normalized mean, which account for outliers
	float normStdev;	// The normalized standard deviation
	float normAbdev;	// The normalized absolute deviation
	uint64_t normMax;	// The normalized maximum value
} test_stat_t;

/**
 * This function returns the mean of the sample set.
 */
static inline
float fipc_test_stat_get_mean ( uint64_t* sample_set, uint64_t sample_size )
{
	register float sum;
	register uint64_t i;

	for ( sum = 0, i = 0; i < sample_size; ++i )
	{
		sum += sample_set[i];
	}

	return sum / (float) sample_size;
}

/**
 * This function returns the basic statistics about the sample set
 */
static inline
int fipc_test_stat_get_stats ( uint64_t* sample_set, uint64_t sample_size, test_stat_t* stat )
{
	// Error Checking
	if ( stat == NULL )
		return -1;

	stat->mean = fipc_test_stat_get_mean ( sample_set, sample_size );
	stat->N    = sample_size;

	stat->min = sample_set[0];
	stat->max = sample_set[0];
	stat->normMax  = sample_set[0];
	stat->outliers = 0;

	register float stdevSum = 0;
	register float abdevSum = 0;
	register float normSum = 0;
	register float normStdevSum = 0;
	register float normAbdevSum = 0;
	register uint64_t i;

	for ( i = 0; i < sample_size; ++i )
	{
		uint64_t dev = ABSOLUTE( sample_set[i]-stat->mean );
		abdevSum += dev;
		stdevSum += dev*dev;

		if ( sample_set[i] < stat->min )
			stat->min = sample_set[i];

		if ( sample_set[i] > stat->max )
			stat->max = sample_set[i];

		if ( sample_set[i] > OUTLIER_THRESHOLD )
		{
			stat->outliers++;
		}
		else
		{
			normSum += sample_set[i];
			normAbdevSum += dev;
			normStdevSum += dev*dev;

			if ( sample_set[i] > stat->normMax )
				stat->normMax = sample_set[i];
		}
	}

	stat->stdev = sqrt( stdevSum / (float) sample_size );
	stat->abdev = abdevSum / (float) sample_size;
	stat->normMean = normSum / (float) ( sample_size - stat->outliers );
	stat->normStdev = sqrt( normStdevSum / (float) ( sample_size - stat->outliers ) );
	stat->normAbdev = normAbdevSum / (float) ( sample_size - stat->outliers );

	return 0;
}

/**
 * This function returns the value with the specified zScore.
 */
static inline
uint64_t fipc_test_stat_zrange_value ( test_stat_t* stat, float zScore )
{
	return stat->normMean + zScore*stat->normStdev;
}

/**
 * This function counts the number of data points in the zScore range.
 */
static inline
uint64_t fipc_test_stat_count_in_zrange ( uint64_t* sample_set, uint64_t sample_size, test_stat_t* stat, float zScoreMin, float zScoreMax )
{
	uint64_t value_below = fipc_test_stat_zrange_value( stat, zScoreMin );
	uint64_t value_above = fipc_test_stat_zrange_value( stat, zScoreMax );

	uint64_t i;
	uint64_t count;

	for ( count = 0, i = 0; i < sample_size; ++i )
		if ( sample_set[i] >= value_below && sample_set[i] <  value_above )
			++count;

	return count;
}

/**
 * This function prints one bar in a histogram corresponding to the range given by zScoreMin and zScoreMax.
 */
static inline
int fipc_test_stat_print_zrange_bar ( uint64_t* sample_set, uint64_t sample_size, test_stat_t* stat, float zScoreMin, float zScoreMax )
{
	char bar[51];
	bar[50] = '\0';

	uint64_t count   = fipc_test_stat_count_in_zrange( sample_set, sample_size, stat, zScoreMin, zScoreMax );
	uint64_t X_value = sample_size / 50;
	uint64_t X_count = count / X_value;

	memset( bar, 'X', X_count );
	memset( bar + X_count, ' ', 50 - X_count );

	printf ( "%s : %lu", bar, count );
	return 0;
}

/**
 * This function prints a histogram of the data.
 */
static inline
int fipc_test_stat_print_zhistogram ( uint64_t* sample_set, uint64_t sample_size, test_stat_t* stat )
{

	uint64_t value_below = 0;
	uint64_t value_above = fipc_test_stat_zrange_value( stat, -3 );

	printf ( "%lu -> %lu\t: ", value_below, value_above );
	fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, -50, -3 );
	printf( "\n" );

	float i;
	for ( i = -3; i < -0.5; i += 0.5 )
	{
		value_below = fipc_test_stat_zrange_value( stat, i );
		value_above = fipc_test_stat_zrange_value( stat, i+0.5 );

		printf ( "%lu -> %lu\t: ", value_below, value_above );
		fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, i, i+0.5 );
		printf( "\n" );
	}

	value_below = fipc_test_stat_zrange_value( stat, -0.5 );
	value_above = fipc_test_stat_zrange_value( stat, 0.5 );

	printf ( "%lu -> %lu\t: ", value_below, value_above );
	fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, -0.5, 0.5 );
	printf( "\n" );

	for ( i = 0.5; i < 3; i += 0.5 )
	{
		value_below = fipc_test_stat_zrange_value( stat, i );
		value_above = fipc_test_stat_zrange_value( stat, i+0.5 );

		printf ( "%lu -> %lu\t: ", value_below, value_above );
		fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, i, i+0.5 );
		printf( "\n" );
	}

	value_below = fipc_test_stat_zrange_value( stat, 4 );
	value_above = stat->max;

	printf ( "%lu -> %lu\t: ", value_below, value_above );
	fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, 4, stat->max );
	printf( "\n" );

	return 0;
}

/**
 * This function prints statistics of the sample set splits to stdin.
 */
int fipc_test_stat_print_info_split ( uint64_t* sample_set, uint64_t sample_size )
{
	// Error Check
	if ( sample_set == NULL || sample_size == 0 )
		return -1;

	uint64_t i25 = sample_size/4;
	uint64_t i50 = sample_size/2;
	uint64_t i75 = 3*(sample_size/4);
	uint64_t i00 = sample_size;

	test_stat_t stats[4];
	fipc_test_stat_get_stats( sample_set +   0, i25 -   0, &stats[0] );
	fipc_test_stat_get_stats( sample_set + i25, i50 - i25, &stats[1] );
	fipc_test_stat_get_stats( sample_set + i50, i75 - i50, &stats[2] );
	fipc_test_stat_get_stats( sample_set + i75, i00 - i75, &stats[3] );

	uint64_t i;
	for ( i = 0; i < 4; ++i )
	{
		// Print Statistics
		printf ( "-------------------------------------------------------------------------------\n" );
		printf ( "Average value: %f\n", stats[i].mean );
		printf ( "Minimum value: %lu\n", stats[i].min );
		printf ( "Maximum value: %lu\n", stats[i].max );
		printf ( "Standard Deviation: %f\n", stats[i].stdev );
		printf ( "Absolute Deviation: %f\n", stats[i].abdev );
		printf ( "Outlier Count: %lu\n", stats[i].outliers );
		printf ( "Normalized Mean: %f\n", stats[i].normMean );
		printf ( "Normalized Standard Deviation: %f\n", stats[i].normStdev );
		printf ( "Normalized Absolute Deviation: %f\n", stats[i].normAbdev );
		printf ( "Normalized Maximum value: %lu\n", stats[i].normMax );
		printf ( "\n" );
		fipc_test_stat_print_zhistogram( sample_set, sample_size, &stats[i] );
		printf ( "\n" );
		printf ( "Summary:\nnorm mean:\t%f;\tnorm abs dev:\t%f\nmin:\t\t%lu;\t\tmax:\t\t%lu\n", 
					stats[i].normMean, stats[i].normAbdev, stats[i].min, stats[i].max );
		printf ( "-------------------------------------------------------------------------------\n" );
	}

	return 0;
}

/**
 * This function prints statistics of the sample set to stdin.
 */
int fipc_test_stat_print_info ( uint64_t* sample_set, uint64_t sample_size )
{
	// Error Check
	if ( sample_set == NULL || sample_size == 0 )
		return -1;

	test_stat_t stats;
	fipc_test_stat_get_stats( sample_set, sample_size, &stats );

	// Print Statistics
	printf ( "-------------------------------------------------------------------------------\n" );
	printf ( "Average value: %f\n", stats.mean );
	printf ( "Minimum value: %lu\n", stats.min );
	printf ( "Maximum value: %lu\n", stats.max );
	printf ( "Standard Deviation: %f\n", stats.stdev );
	printf ( "Absolute Deviation: %f\n", stats.abdev );
	printf ( "Outlier Count: %lu\n", stats.outliers );
	printf ( "Normalized Mean: %f\n", stats.normMean );
	printf ( "Normalized Standard Deviation: %f\n", stats.normStdev );
	printf ( "Normalized Absolute Deviation: %f\n", stats.normAbdev );
	printf ( "Normalized Maximum value: %lu\n", stats.normMax );
	printf ( "\n" );
	fipc_test_stat_print_zhistogram( sample_set, sample_size, &stats );
	printf ( "\n" );
	printf ( "Summary:\nnorm mean:\t%f;\tnorm abs dev:\t%f\nmin:\t\t%lu;\t\tmax:\t\t%lu\n", 
				stats.normMean, stats.normAbdev, stats.min, stats.max );
	printf ( "-------------------------------------------------------------------------------\n" );

	return 0;
}

#endif
