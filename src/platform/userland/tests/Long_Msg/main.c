/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 *
 * This test measures the latency of a long message going through a pipeline
 * of N processors.
 *
 * NOTE: This test assumes an x86 architecture.
 *
 * NOTE: This test assumes a computer with 2-32 processing units.
 */

#include "test.h"

int main ( void )
{
	///////////// Setup Processes

	// Create n processes: P0, P1, P2, ..., Pn-1
	uint64_t rank;
	for ( rank = 0; rank < NUM_PROCESSORS - 1; rank++ )
	{
		pid_t child = fork();
		if ( child == 0 )
			break;
	}

	// Pin Px to processor cpu_map[x]
	fipc_test_thread_pin_this_process_to_CPU( cpu_map[rank] );

	///////////// Setup IPC Mechanism

	fipc_init();

	header_t*  chan = NULL;
	message_t* tx   = NULL;
	message_t* rx   = NULL;

	fipc_test_shm_create_half_channel( CHANNEL_ORDER, &chan, shm_keys[rank], FIPC_TEST_TRANSMIT );
	fipc_test_shm_create_half_channel( CHANNEL_ORDER, &chan, shm_keys[rank == 0 ? NUM_PROCESSORS-1 : rank - 1], FIPC_TEST_RECEIVE );

	if ( chan == NULL )
	{
		fprintf( stderr, "%s\n", "Error when creating channels" );
		return -1;
	}

	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	
	///////////// Throughput Test

	#ifndef FIPC_TEST_TIME_PER_TRANSACTION
		start = RDTSC_START();
		uint64_t throughput_time = 0;
	#else
		register uint64_t* throughput_times = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	#endif

	if ( rank == 0 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				start = RDTSC_START();
			#endif

			fipc_test_blocking_long_send_start( chan, &tx, MSG_LENGTH );
			fipc_send_msg_end ( chan, tx );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				throughput_times[transaction_id] = end - start;
			#endif
		}
	}
	else if ( rank == NUM_PROCESSORS - 1 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				start = RDTSC_START();
			#endif

			fipc_test_blocking_recv_start( chan, &rx );
			fipc_recv_msg_end( chan, rx );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				throughput_times[transaction_id] = end - start;
			#endif
		}
	}
	else
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				start = RDTSC_START();
			#endif

			fipc_test_blocking_recv_start( chan, &rx );
			fipc_recv_msg_end( chan, rx );

			fipc_test_blocking_long_send_start( chan, &tx, MSG_LENGTH );
			fipc_send_msg_end ( chan, tx );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				throughput_times[transaction_id] = end - start;
			#endif
		}
	}

	#ifndef FIPC_TEST_TIME_PER_TRANSACTION
		end = RDTSCP();
		throughput_time = (start - end) / TRANSACTIONS
	#endif

	///////////// Latency Test

	register uint64_t* latency_times = NULL;
	register uint64_t  latency_index = 0;
	if ( rank == 0 )   latency_times = malloc( TRANSACTIONS * sizeof( uint64_t ) );

	if ( rank == 0 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			start = RDTSC_START();

			fipc_test_blocking_long_send_start( chan, &tx, MSG_LENGTH );
			tx->regs[9] = start;
			fipc_send_msg_end ( chan, tx );

			if ( fipc_recv_msg_start( chan, &rx ) == -EWOULDBLOCK )
				continue;

			start = rx->regs[9];
			fipc_recv_msg_end( chan, rx );

			latency_times[ latency_index++ ] = RDTSCP() - start;
		}

		while ( latency_index < TRANSACTIONS )
		{
			fipc_test_blocking_recv_start( chan, &rx )
			start = rx->regs[9];
			fipc_recv_msg_end( chan, rx );

			latency_times[ latency_index++ ] = RDTSCP() - start;
		}
	}
	else
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			fipc_test_blocking_recv_start( chan, &rx );
			start = rx->regs[9];
			fipc_recv_msg_end( chan, rx );

			fipc_test_blocking_long_send_start( chan, &tx, MSG_LENGTH );
			tx->regs[9] = start;
			fipc_send_msg_end ( chan, tx );
		}
	}

	///////////// Synchronized display of test metrics

	if ( rank == 0 )
	{
		#ifdef FIPC_TEST_TIME_PER_TRANSACTION
			printf( "Process %lu's stats\n", rank );
			fipc_test_stat_print_info( throughput_times, TRANSACTIONS );
		#endif

		printf( "Latency Statistics\n" );
		fipc_test_stat_print_info( latency_times, TRANSACTIONS );

		fipc_test_blocking_send_start( chan, &tx );
		fipc_send_msg_end ( chan, tx );
	}
	else if ( rank == NUM_PROCESSORS-1 )
	{
		fipc_test_blocking_recv_start( chan, &rx );
		fipc_recv_msg_end( chan, rx );

		#ifndef FIPC_TEST_TIME_PER_TRANSACTION
			printf( "Average cycles to send one message through the pipeline: %lu\n", throughput_time );
		#else
			printf( "Process %lu's stats\n", rank );
			fipc_test_stat_print_info( throughput_times, TRANSACTIONS );
		#endif
	}
	else
	{
		fipc_test_blocking_recv_start( chan, &rx );
		message_t temp = *rx;
		fipc_recv_msg_end( chan, rx );

		#ifdef FIPC_TEST_TIME_PER_TRANSACTION
			printf( "Process %lu's stats\n", rank );
			fipc_test_stat_print_info( throughput_times, TRANSACTIONS );
		#endif

		fipc_test_blocking_send_start( chan, &tx );
		*tx = temp;
		fipc_send_msg_end( chan, tx );
	}

	///////////// Clean Up

	fipc_test_shm_free_half_channel( chan );
	fipc_test_shm_unlink( shm_keys[rank] );
	fipc_test_shm_unlink( shm_keys[rank == 0 ? NUM_PROCESSORS-1 : rank - 1] );
	#ifdef FIPC_TEST_TIME_PER_TRANSACTION
		free ( throughput_times );
	#endif

	if ( rank == 0 )
		free ( latency_times );
	
	fipc_fini();
	return 0;
}
