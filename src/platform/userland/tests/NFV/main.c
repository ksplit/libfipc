/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 *
 * This test passes a simulated packet through a series of processes, which
 * represent composed functions. This is done using separate address spaces
 * to isolate the functions.
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
	for ( rank = 0; rank < NUM_PROCESSORS-1; rank++ )
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

	///////////// Test Variables

	register uint64_t  CACHE_ALIGNED transaction_id;
	register uint64_t  CACHE_ALIGNED start;
	register uint64_t  CACHE_ALIGNED end;
	register packet_t* CACHE_ALIGNED packet_ptr = malloc ( sizeof( packet_t ) );

	///////////// Throughput Test

	#ifndef FIPC_TEST_TIME_PER_TRANSACTION
		uint64_t throughput_time = 0;
		start = RDTSC_START();
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

			// Apply function to packet
			packet_ptr = pipe_func[rank]( packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send packet to next stage
			fipc_test_blocking_long_send_start( chan, &tx, LINES_PER_PACKET+1 );
			memcpy( &tx->regs[8], packet_ptr, sizeof( packet_t ) );
			fipc_send_msg_end ( chan, tx );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				throughput_times[transaction_id] = end - start;
			#endif
		}
	}
	else if ( rank == NUM_PROCESSORS-1 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				start = RDTSC_START();
			#endif

			// Get packet from previous stage
			fipc_test_blocking_recv_start( chan, &rx );
			memcpy( packet_ptr, &rx->regs[8], sizeof( packet_t ) );
			fipc_recv_msg_end( chan, rx );

			// Apply function to packet
			packet_ptr = pipe_func[rank]( packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

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

			// Get packet from previous stage
			fipc_test_blocking_recv_start( chan, &rx );
			memcpy( packet_ptr, &rx->regs[8], sizeof( packet_t ) );
			fipc_recv_msg_end( chan, rx );

			// Apply function to packet
			packet_ptr = pipe_func[rank]( packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send packet to next stage
			fipc_test_blocking_long_send_start( chan, &tx, LINES_PER_PACKET+1 );
			memcpy( &tx->regs[8], packet_ptr, sizeof( packet_t ) );
			fipc_send_msg_end ( chan, tx );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				throughput_times[transaction_id] = end - start;
			#endif
		}
	}

	#ifndef FIPC_TEST_TIME_PER_TRANSACTION
		end = RDTSCP();
		throughput_time = (end - start) / TRANSACTIONS;
	#endif

	///////////// Latency Test

	register uint64_t* latency_times = NULL;

	if ( rank == 0 )
	{
		latency_times = malloc( TRANSACTIONS * sizeof( uint64_t ) );

		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			// Start Time
			start = RDTSC_START();

			// Apply function to packet
			packet_ptr = pipe_func[rank]( packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send packet to next stage
			fipc_test_blocking_long_send_start( chan, &tx, LINES_PER_PACKET+1 );
			memcpy( &tx->regs[8], packet_ptr, sizeof( packet_t ) );
			fipc_send_msg_end ( chan, tx );

			// Wait for finished message from the last process
			fipc_test_blocking_recv_start( chan, &rx );
			fipc_recv_msg_end( chan, rx );

			// End time
			end = RDTSCP();
			latency_times[ transaction_id ] = end - start;
		}
	}
	else if ( rank == NUM_PROCESSORS-1 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			// Get packet from previous stage
			fipc_test_blocking_recv_start( chan, &rx );
			memcpy( packet_ptr, &rx->regs[8], sizeof( packet_t ) );
			fipc_recv_msg_end( chan, rx );

			// Apply function to packet
			packet_ptr = pipe_func[rank]( packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send finished message to process 0
			fipc_test_blocking_send_start( chan, &tx );
			fipc_send_msg_end ( chan, tx );
		}
	}
	else
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			// Get packet from previous stage
			fipc_test_blocking_recv_start( chan, &rx );
			memcpy( packet_ptr, &rx->regs[8], sizeof( packet_t ) );
			fipc_recv_msg_end( chan, rx );

			// Apply function to packet
			packet_ptr = pipe_func[rank]( packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send packet to next stage
			fipc_test_blocking_long_send_start( chan, &tx, LINES_PER_PACKET+1 );
			memcpy( &tx->regs[8], packet_ptr, sizeof( packet_t ) );
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
			double bytesPerSecond = ((double) 1 / ( (double) throughput_time / (FIPC_CACHE_LINE_SIZE*MSG_LENGTH) ))*2200000000;
			printf( "Average cycles to send one message through the pipeline: %lu\n", throughput_time );
			printf( "Throughput: %f bytes/second\n", bytesPerSecond );
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

	free ( packet_ptr );
	
	fipc_fini();
	return 0;
}
