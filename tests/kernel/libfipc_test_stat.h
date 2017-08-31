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
#define INF  9223372036854775807
#define NINF -9223372036854775807
#define INF_32  2147483647
#define NINF_32 -2147483648

#define OUTLIER_ORDER 5

typedef struct stats_t
{
	uint32_t N;			// The size of the sample set

	int64_t  mean;		// The mean of the data
	int64_t stdev;		// The standard deviation
	int64_t abdev;		// The mean absolute deviation
	int32_t  min;		// The minimum value
	int32_t  max;		// The maximum value

	int64_t  tolerance;	// The tolerance level used in classifying outliers
	uint32_t outliers;	// The number of data points classified as outliers

	int64_t  norm_mean;		// The mean of the data without the outliers
	int64_t norm_stdev;	// The standard deviation without the outliers
	int64_t norm_abdev;	// The mean absolute deviation without the outliers
	int32_t  norm_min;		// The minimum value without the outliers
	int32_t  norm_max;		// The maximum value without the outliers

	int stdev_overflow_flag;		// Gets set if stdev overflows
	int norm_stdev_overflow_flag;	// Gets set if norm_stdev overflows

} stats_t;

/**
 * int_sqrt - rough approximation to sqrt
 * @Author   : Abhiram Balasubramanian
 * @x: integer of which to calculate the sqrt
 *
 * A very rough approximation to the sqrt() function.
 */
static inline
unsigned long int_sqrt_local ( unsigned long x )
{
	unsigned long b, m, y = 0;

	if (x <= 1)
		return x;

	m = 1UL << (BITS_PER_LONG - 2); 
	while (m != 0)
	{
		b = y + m;
		y >>= 1;

		if (x >= b)
		{
			x -= b;
			y += m;
		}
		m >>= 2;
	}

	return y;
}

/**
 * This function returns an approximated mean of the sample set.
 */
static inline
int64_t fipc_test_stat_get_mean ( int32_t* sample_set, uint32_t sample_size )
{
	register int64_t sum;
	register uint32_t i;

	for ( sum = 0, i = 0; i < sample_size; ++i )
	{
		sum += (int64_t) sample_set[i];
	}

	return (sum / (int64_t) sample_size);
}

/**
 * This function returns a tolerance level, the suggested max deviation
 * from the mean before being classified as an outlier.
 */
static inline
int64_t fipc_test_stat_get_tolerance ( int32_t* sample_set, uint32_t sample_size )
{
	register int64_t sum;
	register int64_t mean;
	register uint64_t i;

	// For a quick sketch idea of the data, only go through 1/16 of it.
	if ( sample_size > 16 )
		sample_size >>= 4;

	mean = fipc_test_stat_get_mean ( sample_set, sample_size );

	for ( sum = 0, i = 0; i < sample_size; ++i )
	{
		sum += ABSOLUTE( (int64_t)sample_set[i] - mean );
	}

	int64_t sketch_abdev = (sum / (int64_t) sample_size);

	return OUTLIER_ORDER * sketch_abdev;
}

/**
 * This function populates the stat data structure with statistics.
 */
int fipc_test_stat_calculate_stats ( int32_t* sample_set, uint32_t sample_size, stats_t* stat )
{
	// Error Checking
	if ( stat == NULL || sample_size == 0 )
		return -1;

	stat->N    = sample_size;
	stat->mean = fipc_test_stat_get_mean ( sample_set, sample_size );
	stat->tolerance = fipc_test_stat_get_tolerance ( sample_set, sample_size );

	int32_t upper_thresh = stat->tolerance >= INF_32 ? INF_32 : stat->mean + stat->tolerance;
	int32_t lower_thresh = stat->tolerance >= INF_32 ? NINF_32 : stat->mean - stat->tolerance;

	// Temporary Values
	stat->min = INF_32;
	stat->max = NINF_32;
	stat->norm_min = INF_32;
	stat->norm_max = NINF_32;
	stat->outliers = 0;
	stat->stdev_overflow_flag = 0;
	stat->norm_stdev_overflow_flag = 0;

	register int64_t stdevSum = 0;
	register int64_t abdevSum = 0;
	register int64_t normSum = 0;
	register int64_t normStdevSum = 0;
	register int64_t normAbdevSum = 0;
	register uint32_t i;


	for ( i = 0; i < sample_size; ++i )
	{
		int64_t dev = ABSOLUTE( (int64_t)sample_set[i] - stat->mean );
		abdevSum += dev;

		if ( stat->stdev_overflow_flag == 1 || stdevSum > stdevSum + dev*dev )
			stat->stdev_overflow_flag = 1;
		else
			stdevSum += dev*dev;

		if ( sample_set[i] < stat->min )
			stat->min = sample_set[i];

		if ( sample_set[i] > stat->max )
			stat->max = sample_set[i];

		if ( sample_set[i] > upper_thresh || sample_set[i] < lower_thresh )
		{
			stat->outliers++;
		}
		else
		{
			normSum += sample_set[i];

			if ( sample_set[i] > stat->norm_max )
				stat->norm_max = sample_set[i];

			if ( sample_set[i] < stat->norm_min )
				stat->norm_min = sample_set[i];
		}
	}

	stat->stdev     = int_sqrt_local( stdevSum / (int64_t) sample_size );
	stat->abdev     = ( abdevSum / (int64_t) sample_size);
	stat->norm_mean = ( normSum / (int64_t) ( sample_size - stat->outliers ) );

	for ( i = 0; i < sample_size; ++i )
	{
		if ( sample_set[i] <= upper_thresh && sample_set[i] >= lower_thresh )
		{
			uint64_t dev = ABSOLUTE( (int64_t)sample_set[i] - stat->norm_mean );
			normAbdevSum += dev;

			if ( stat->norm_stdev_overflow_flag == 1 || normStdevSum > normStdevSum + dev*dev )
				stat->norm_stdev_overflow_flag = 1;
			else
				normStdevSum += dev*dev;
		}
	}

	stat->norm_stdev = int_sqrt_local( normStdevSum / (uint64_t) ( sample_size - stat->outliers ) );
	stat->norm_abdev = ( normAbdevSum / (uint64_t) ( sample_size - stat->outliers ) );
	return 0;
}

/**
 * This function returns the value with the specified zScore.
 */
static inline
int32_t fipc_test_stat_zrange_value ( stats_t* stat, int64_t zScore )
{
	if ( stat->norm_stdev_overflow_flag == 1 )
		return 0;

	int64_t value = stat->norm_mean + zScore*stat->norm_stdev;

	if ( value <= NINF_32 || value >= INF_32 )
		return 0;

	return (int32_t)value;
}

/**
 * This function returns the zScore of the specified value.
 */
static inline
int64_t fipc_test_stat_zscore_value ( stats_t* stat, int32_t value )
{
	if ( stat->norm_stdev == 0 || stat->norm_stdev_overflow_flag == 1 )
		return 0;
	int64_t zScore = ( (int64_t)value - stat->norm_mean ) / stat->norm_stdev;

	// rounds zScore to nearest integer rather than floor
	if ( ((int64_t)value - stat->norm_mean) % stat->norm_stdev >= stat->norm_stdev / 2 )
		zScore += zScore < 0 ? -1 : 1;

	return zScore;
}

/**
 * This function counts the number of data points in the zScore range (inclusive).
 */
static inline
uint32_t fipc_test_stat_count_in_range ( int32_t* sample_set, uint32_t sample_size, int64_t min, int64_t max )
{
	register uint32_t i;
	register uint32_t count;

	for ( count = 0, i = 0; i < sample_size; ++i )
		if ( (min == max && sample_set[i] == min) || (sample_set[i] >= min && sample_set[i] < max) )
			++count;

	return count;
}

/**
 * This function prints one bar in a histogram corresponding to the range given by zScoreMin and zScoreMax.
 */
static inline
int fipc_test_stat_print_zrange_bar ( int32_t* sample_set, uint32_t sample_size, stats_t* stat, int64_t zScoreMin, int64_t zScoreMax )
{
	char bar[33];

	bar[32] = '\0';

	int32_t value_below = fipc_test_stat_zrange_value( stat, zScoreMin );
	int32_t value_above = fipc_test_stat_zrange_value( stat, zScoreMax );

	// Calculate the number of 'X's
	uint32_t count   = fipc_test_stat_count_in_range( sample_set, sample_size, value_below, value_above );
	uint32_t X_value = sample_size >> 5 == 0 ? 1 : sample_size >> 5;
	uint32_t X_count = count / X_value;

	// Construct the bar string
	memset( bar, 'X', X_count );
	memset( bar + X_count, ' ', 32 - X_count );

	pr_err ( "%13d -> %13d : %s : %12d\n", value_below, value_above, bar, count );
	return 0;
}

/**
 * This function prints a histogram of the data.
 */
static inline
int fipc_test_stat_print_zhistogram ( int32_t* sample_set, uint32_t sample_size, stats_t* stat )
{
	int64_t zNINF = fipc_test_stat_zscore_value( stat, NINF_32 );
	int64_t zINF  = fipc_test_stat_zscore_value( stat, INF_32  );

	int64_t i = fipc_test_stat_zscore_value( stat, stat->min );
	i = i < -3 ? -3 : i; // i = max(-3, min)
	i = i < zNINF ? zNINF : i; // i = max(i, z(NINF))

	int64_t maxZ = fipc_test_stat_zscore_value( stat, stat->max );
	maxZ = maxZ > 3 ? 3 : maxZ; // maxZ = min(3, max)
	maxZ = maxZ > zINF ? zINF : maxZ; // maxZ = min(maxZ, z(INF))

	if ( i >= maxZ )
		fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, -1, 1 );

	for ( ; i < maxZ; i++ )
		fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, i, i+1 );

	return 0;
}

/**
 * This function prints statistics of the sample set to stdout.
 */
int fipc_test_stat_print_stats ( int32_t* sample_set, uint32_t sample_size, stats_t* stat )
{
	pr_err ( "-------------------------------------------------------------------------------\n" );
	pr_err ( "Sample Size             : %u\n", stat->N );
	pr_err ( "*Average value          : %lld\n", stat->mean );
	pr_err ( "Minimum value           : %d\n", stat->min );
	pr_err ( "Maximum value           : %d\n", stat->max );

	if ( stat->stdev_overflow_flag == 0 )
		pr_err ( "*Standard Deviation     : %lld\n", stat->stdev );
	else
		pr_err ( "*Standard Deviation     : overflow\n" );

	pr_err ( "*Mean Absolute Deviation: %lld\n", stat->abdev );
	pr_err ( "\n" );

	pr_err ( "Outlier Count   : %u\n", stat->outliers );
	if ( stat->outliers > 0 )
	{
		pr_err ( "Without Outliers:\n");
		pr_err ( "  *Average value          : %lld\n", stat->norm_mean );
		pr_err ( "  Minimum value           : %d\n", stat->norm_min );
		pr_err ( "  Maximum value           : %d\n", stat->norm_max );
		if ( stat->stdev_overflow_flag == 0 )
			pr_err ( "  *Standard Deviation     : %lld\n", stat->norm_stdev );
		else
			pr_err ( "  *Standard Deviation     : overflow\n" );
		pr_err ( "  *Mean Absolute Deviation: %lld\n", stat->norm_abdev );
	}
	pr_err ( "\n" );

	fipc_test_stat_print_zhistogram( sample_set, sample_size, stat );
	pr_err ( "\n" );

	pr_err ( "Summary:\n");
	pr_err ( "min : %14d || max : %14d || *mean : %14lld\n", stat->min, stat->max, stat->mean );

	if ( stat->outliers > 0 )
		pr_err ( "Nmin: %14d || Nmax: %14d || *Nmean: %14lld\n", stat->norm_min, stat->norm_max, stat->norm_mean );

	pr_err ( "\n" );

	pr_err ( "* - Approximated using integer arithmetic\n");
	pr_err ( "-------------------------------------------------------------------------------\n" );

	return 0;
}

/**
 * This function prints a specified amount of raw data.
 */
int fipc_test_stat_print_raw ( int32_t* sample_set, uint32_t sample_size, uint32_t print_count )
{
	if ( print_count > sample_size )
		print_count = sample_size;

	int i;
	for ( i = 0; i < print_count; ++i )
		pr_err ( "%ld.\t %ld\n", i, sample_set[i] );

	return 0;
}

/**
 * This function calculates and prints statistics of the given sample set.
 */
int fipc_test_stat_get_and_print_stats ( int32_t* sample_set, uint32_t sample_size )
{
	int error_code = 0;

	stats_t stats;
	error_code = fipc_test_stat_calculate_stats ( sample_set, sample_size, &stats );

	if ( error_code != 0 )
		return error_code;

	return fipc_test_stat_print_stats ( sample_set, sample_size, &stats );
}

#endif
