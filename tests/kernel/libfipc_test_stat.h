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

#define DTOC(x)     ((char)('0' + (x)))

typedef struct stats_t
{
	uint32_t N;			// The size of the sample set

	int32_t  mean;		// The mean of the data
	uint32_t stdev;		// The standard deviation
	uint32_t abdev;		// The mean absolute deviation
	int32_t  min;		// The minimum value
	int32_t  max;		// The maximum value

	int64_t  tolerance;	// The tolerance level used in classifying outliers
	uint32_t outliers;	// The number of data points classified as outliers

	int32_t  norm_mean;		// The mean of the data without the outliers
	uint32_t norm_stdev;	// The standard deviation without the outliers
	uint32_t norm_abdev;	// The mean absolute deviation without the outliers
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
int32_t fipc_test_stat_get_mean ( int32_t* sample_set, uint32_t sample_size )
{
	register int64_t sum;
	register uint32_t i;

	for ( sum = 0, i = 0; i < sample_size; ++i )
	{
		sum += (int64_t) sample_set[i];
	}

	return (int32_t)(sum / (int64_t) sample_size);
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

	return 5 * sketch_abdev;
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

	register uint64_t stdevSum = 0;
	register uint64_t abdevSum = 0;
	register int64_t normSum = 0;
	register uint64_t normStdevSum = 0;
	register uint64_t normAbdevSum = 0;
	register uint32_t i;


	for ( i = 0; i < sample_size; ++i )
	{
		uint64_t dev = ABSOLUTE( sample_set[i] - stat->mean );
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

	stat->stdev     = int_sqrt_local( stdevSum / (uint64_t) sample_size );
	stat->abdev     = (uint32_t)( abdevSum / (uint64_t) sample_size);
	stat->norm_mean = (uint32_t)( normSum / (uint64_t) ( sample_size - stat->outliers ) );

	for ( i = 0; i < sample_size; ++i )
	{
		if ( sample_set[i] <= upper_thresh && sample_set[i] >= lower_thresh )
		{
			uint64_t dev = ABSOLUTE( sample_set[i] - stat->norm_mean );
			normAbdevSum += dev;

			if ( stat->norm_stdev_overflow_flag == 1 || normStdevSum > normStdevSum + dev*dev )
				stat->norm_stdev_overflow_flag = 1;
			else
				normStdevSum += dev*dev;
		}
	}

	stat->norm_stdev = int_sqrt_local( normStdevSum / (uint64_t) ( sample_size - stat->outliers ) );
	stat->norm_abdev = (uint32_t)( normAbdevSum / (uint64_t) ( sample_size - stat->outliers ) );
	return 0;
}

/**
 * This function returns the value with the specified zScore.
 */
// static inline
// uint64_t fipc_test_stat_zrange_value ( stats_t* stat, double zScore )
// {
// 	return stat->norm_mean + zScore*stat->norm_stdev;
// }

/**
 * This function returns the zScore with the specified value.
 */
// static inline
// double fipc_test_stat_zscore_value ( stats_t* stat, int64_t value )
// {
// 	if ( stat->norm_stdev == 0 )
// 		return 0;

// 	return (double)(value - stat->norm_mean) / stat->norm_stdev;
// }

/**
 * This function counts the number of data points in the zScore range (inclusive).
 */
// static inline
// uint64_t fipc_test_stat_count_in_range ( int32_t* sample_set, uint32_t sample_size, int64_t min, int64_t max )
// {
// 	register uint64_t i;
// 	register uint64_t count;

// 	for ( count = 0, i = 0; i < sample_size; ++i )
// 		if ( sample_set[i] >= min && sample_set[i] <= max )
// 			++count;

// 	return count;
// }

/**
 * This function truncates (or pads) the given int64_t into a string with given width
 */
// static inline
// int fipc_test_stat_truncate ( int64_t value, char* buf, uint64_t width )
// {
// 	// Error Checking
// 	if ( width == 0 )
// 		return -1;

// 	// Negative Symbol
// 	if ( value < 0 )
// 	{
// 		buf[0] = '-';
// 		return fipc_test_stat_truncate ( value <= NINF ? INF : -value, ++buf, --width );
// 	}

// 	uint64_t buf_index   = 0;
// 	uint64_t digit_index = 0;

// 	char digits[21];
// 	memset( digits, 0, 21 );

// 	// Turn value into char array
// 	while ( value > 0 && digit_index < 20 )
// 	{
// 		int64_t digit = value % 10;
// 		value = value / 10;
// 		digits[digit_index++] = DTOC(digit);
// 	}

// 	// Reverse it into buf
// 	while ( width > 0 && digit_index >= 1 )
// 	{
// 		buf[buf_index++] = digits[--digit_index];
// 		--width;
// 	}

// 	// Append suffix, if needed
// 	if ( digit_index >= 1 )
// 	{
// 		int chars_deleted = 3 - (digit_index % 3);

// 		buf_index   -= chars_deleted;
// 		width       += chars_deleted;
// 		digit_index += chars_deleted;

// 		if ( buf_index < 0 )
// 			return -1;

// 		char suffix = ' ';

// 		if ( digit_index >= 18 )
// 			suffix = 'E';
// 		else if ( digit_index >= 15 )
// 			suffix = 'P';
// 		else if ( digit_index >= 12 )
// 			suffix = 'T';
// 		else if ( digit_index >= 9 )
// 			suffix = 'G';
// 		else if ( digit_index >= 6 )
// 			suffix = 'M';
// 		else if ( digit_index >= 3 )
// 			suffix = 'K';

// 		buf[buf_index++] = suffix;
// 		width--;
// 	}

// 	// Pad with spaces
// 	while ( width > 0 )
// 	{
// 		buf[buf_index++] = ' ';
// 		--width;
// 	}

// 	return 0;
// }

/**
 * This function prints one bar in a histogram corresponding to the range given by zScoreMin and zScoreMax.
 */
// static inline
// int fipc_test_stat_print_zrange_bar ( int32_t* sample_set, uint32_t sample_size, stats_t* stat, double zScoreMin, double zScoreMax )
// {
// 	char bar[33];
// 	char count_str[13];
// 	char value_below_str[14];
// 	char value_above_str[14];

// 	bar[32]             = '\0';
// 	count_str[12]       = '\0';
// 	value_below_str[13] = '\0';
// 	value_above_str[13] = '\0';

// 	int64_t value_below = fipc_test_stat_zrange_value( stat, zScoreMin );
// 	int64_t value_above = fipc_test_stat_zrange_value( stat, zScoreMax );

// 	// Calculate the number of 'X's
// 	uint64_t count   = fipc_test_stat_count_in_range( sample_set, sample_size, value_below, value_above );
// 	uint64_t X_value = sample_size >> 5 == 0 ? 1 : sample_size >> 5;
// 	uint64_t X_count = count / X_value;

// 	// Construct the bar string
// 	memset( bar, 'X', X_count );
// 	memset( bar + X_count, ' ', 32 - X_count );

// 	fipc_test_stat_truncate( count, count_str, 12 );
// 	fipc_test_stat_truncate( value_below, value_below_str, 13 );
// 	fipc_test_stat_truncate( value_above, value_above_str, 13 );


// 	pr_err ( "%s -> %s : %s : %s\n", value_below_str, value_above_str, bar, count_str );
// 	return 0;
// }

/**
 * This function prints a histogram of the data.
 */
// static inline
// int fipc_test_stat_print_zhistogram ( int32_t* sample_set, uint32_t sample_size, stats_t* stat )
// {
// 	double zNINF = fipc_test_stat_zscore_value( stat, NINF );
// 	double zINF  = fipc_test_stat_zscore_value( stat, INF  );

// 	double i = fipc_test_stat_zscore_value( stat, stat->min );
// 	i = i < -3 ? -3 : i; // i = max(-3, min)
// 	i = i < zNINF ? zNINF : i; // i = max(i, z(NINF))

// 	double maxZ = fipc_test_stat_zscore_value( stat, stat->max );
// 	maxZ = maxZ > 3 ? 3 : maxZ; // maxZ = min(3, max)
// 	maxZ = maxZ > zINF ? zINF : maxZ; // maxZ = min(maxZ, z(INF))

// 	if ( i >= maxZ )
// 		fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, -0.5, 0.5 );

// 	for ( ; i < maxZ; i += 0.5 )
// 		fipc_test_stat_print_zrange_bar( sample_set, sample_size, stat, i, ( i+0.5 <= maxZ ? i+0.5 : maxZ ) );

// 	return 0;
// }

/**
 * This function prints statistics of the sample set to stdout.
 */
	uint32_t N;			// The size of the sample set

	int32_t mean;		// The mean of the data
	uint32_t stdev;		// The standard deviation
	uint32_t abdev;		// The mean absolute deviation
	int32_t min;		// The minimum value
	int32_t max;		// The maximum value

	int64_t tolerance;	// The tolerance level used in classifying outliers
	uint32_t outliers;	// The number of data points classified as outliers

	int32_t norm_mean;	// The mean of the data without the outliers
	uint32_t norm_stdev;	// The standard deviation without the outliers
	uint32_t norm_abdev;	// The mean absolute deviation without the outliers
	int32_t norm_min;	// The minimum value without the outliers
	int32_t norm_max;	// The maximum value without the outliers

	int stdev_overflow_flag;		// Gets set if stdev overflows
	int norm_stdev_overflow_flag;	// Gets set if norm_stdev overflows

int fipc_test_stat_print_stats ( int32_t* sample_set, uint32_t sample_size, stats_t* stat )
{
	pr_err ( "-------------------------------------------------------------------------------\n" );
	pr_err ( "Sample Size             : %u\n", stat->N );
	pr_err ( "Average value*          : %d\n", stat->mean );
	pr_err ( "Minimum value           : %d\n", stat->min );
	pr_err ( "Maximum value           : %d\n", stat->max );
	pr_err ( "Standard Deviation*     : %u\n", stat->stdev );
	pr_err ( "Mean Absolute Deviation*: %u\n", stat->abdev );
	pr_err ( "\n" );

	pr_err ( "Outlier Count   : %u\n", stat->outliers );
	if ( stat->outliers > 0 )
	{
		pr_err ( "Without Outliers:\n");
		pr_err ( "  Average value*          : %d\n", stat->norm_mean );
		pr_err ( "  Minimum value           : %d\n", stat->norm_min );
		pr_err ( "  Maximum value           : %d\n", stat->norm_max );
		pr_err ( "  Standard Deviation*     : %u\n", stat->norm_stdev );
		pr_err ( "  Mean Absolute Deviation*: %u\n", stat->norm_abdev );
	}
	pr_err ( "\n" );

	//fipc_test_stat_print_zhistogram( sample_set, sample_size, stat );
	pr_err ( "\n" );

	pr_err ( "Summary:\n");

	// char min[15];
	// char max[15];
	// char mean[15];

	// memset( min, 0, 15 );
	// memset( max, 0, 15 );
	// memset( mean, 0, 15 );

	// fipc_test_stat_truncate( stat->min, min, 14 );
	// fipc_test_stat_truncate( stat->max, max, 14 );
	// fipc_test_stat_truncate( stat->mean, mean, 14 );

	pr_err ( "min : %d || max : %d || mean : %d\n", stat->min, stat->max, stat->mean );

	if ( stat->outliers > 0 )
	{
		// memset( min, 0, 15 );
		// memset( max, 0, 15 );
		// memset( mean, 0, 15 );

		// fipc_test_stat_truncate( stat->norm_min, min, 14 );
		// fipc_test_stat_truncate( stat->norm_max, max, 14 );
		// fipc_test_stat_truncate( stat->norm_mean, mean, 14 );

		pr_err ( "Nmin: %d || Nmax: %d || Nmean: %d\n", stat->norm_min, stat->norm_max, stat->norm_mean );
	}

	pr_err ( "\n" );
	pr_err ( "* - Approximated using integer arithmetic\n");
	pr_err ( "-------------------------------------------------------------------------------\n" );

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
