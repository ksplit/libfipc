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

	// Pin Px to processor x
	fipc_test_thread_pin_this_process_to_CPU( rank );

	printf( "Hello1 from process %lu.\n", rank );
	///////////// Setup IPC Mechanism

	fipc_init();

	// Each header is capable of bidirectional communication; however,
	// we are dealing with composed functions, so our messages only
	// move in one direction. Therefore, we use the headers in a unidirectional
	// fashion. Each header's receive buffer talks to the previous process,
	// where each header's transmit buffer talks to the next process.
	//
	// For time synchronization we need a reversed channel.
	// The channel for communication in the forward direction is the forw channel.
	// The channel for communication in the reverse direction is the back channel.
	//
	// NOTE: In this test, P0 will not receive; likewise, Pn-1 will not send

	header_t*  forw = NULL;
	header_t*  back = NULL;

	message_t* txF  = NULL;
	message_t* rxF  = NULL;
	message_t* txB  = NULL;
	message_t* rxB  = NULL;

	if ( rank != NUM_PROCESSORS-1 )
	{
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &forw, shm_keysF[rank], FIPC_TEST_TRANSMIT );
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &back, shm_keysB[rank], FIPC_TEST_RECEIVE );
	}
	if ( rank != 0 )
	{
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &forw, shm_keysF[rank - 1], FIPC_TEST_RECEIVE );
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &back, shm_keysB[rank - 1], FIPC_TEST_TRANSMIT );
	}

	printf( "Hello2 from process %lu.\n", rank );
	///////////// Time Synchronization

	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t CACHE_ALIGNED sync_offset = 0; // Represents the difference in time between P0 and PN

	if ( rank == 0 )
	{
		start = RDTSC_START();
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			fipc_test_blocking_send_start( forw, &txF );
			fipc_send_msg_end ( forw, txF );

			fipc_test_blocking_recv_start( back, &rxB );
			fipc_recv_msg_end( back, rxB );
		}
		end = RDTSCP();

		fipc_test_blocking_send_start( forw, &txF );
		txF->regs[0] = (end - start)/(2*transaction_id); // Cycles to send an empty message through the pipeline
		txF->regs[1] = RDTSC_START();
		fipc_send_msg_end ( forw, txF );
	}
	else if ( rank == NUM_PROCESSORS - 1 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			fipc_test_blocking_recv_start( forw, &rxF );
			fipc_recv_msg_end( forw, rxF );

			fipc_test_blocking_send_start( back, &txB );
			fipc_send_msg_end( back, txB );
		}

		fipc_test_blocking_recv_start( forw, &rxF );
		sync_offset = RDTSC_START() - rxF->regs[1] - rxF->regs[0];
		fipc_recv_msg_end( back, rxF );
		// printf("%lu\n", sync_offset);
	}
	else
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			fipc_test_blocking_recv_start( forw, &rxF );
			fipc_recv_msg_end( forw, rxF );

			fipc_test_blocking_send_start( forw, &txF );
			fipc_send_msg_end( forw, txF );

			fipc_test_blocking_recv_start( back, &rxB );
			fipc_recv_msg_end( back, rxB );

			fipc_test_blocking_send_start( back, &txB );
			fipc_send_msg_end( back, txB );
		}

		fipc_test_blocking_recv_start( forw, &rxF );
		message_t temp = *rxF;
		fipc_recv_msg_end( forw, rxF );

		fipc_test_blocking_send_start( forw, &txF );
		(*txF) = temp;
		fipc_send_msg_end( forw, txF );
	}

	printf( "Hello3 from process %lu.\n", rank );
	///////////// Main Test

	// Setup Packet Space
	packet_t* packet_space;
	fipc_test_shm_get( "FIPC_NFV_PACKET_SPACE", TRANSACTIONS * sizeof( packet_t ), (void**)&packet_space );
	// printf("%lu\n", sizeof( packet_t) );
	
	#ifndef FIPC_TEST_TIME_PER_TRANSACTION
		start = RDTSC_START();
	#else
		register uint64_t* times = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	#endif

	#ifdef FIPC_TEST_LATENCY
		register uint64_t lat_start;
		register uint64_t* latencyTimes;

		if ( rank == NUM_PROCESSORS-1 )
			latencyTimes = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	#endif

	packet_t* packet_ptr;

	if ( rank == 0 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			// Take at most 1 time stamp, if necessary
			#if  defined(FIPC_TEST_TIME_PER_TRANSACTION) || defined(FIPC_TEST_LATENCY)
				uint64_t timestamp = RDTSC_START();
			#endif

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				start = timestamp;
			#endif

			#ifdef FIPC_TEST_LATENCY
				lat_start = timestamp;
			#endif

			// Get the initial packet
			packet_ptr = &packet_space[transaction_id];

			// Apply pipeline function to packet
			packet_ptr = pipe_func[rank]( (int64_t*)packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send a pointer to the packet in a message to the next process
			fipc_test_blocking_send_start( forw, &txF );
			txF->regs[0] = (uint64_t) packet_ptr;

			#ifdef FIPC_TEST_LATENCY
				txF->regs[1] = lat_start;
			#endif

			fipc_send_msg_end ( forw, txF );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				times[transaction_id] = end - start;
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

			// Receive message and store packet pointer locally
			fipc_test_blocking_recv_start( forw, &rxF );
			packet_ptr = (packet_t*) rxF->regs[0];

			#ifdef FIPC_TEST_LATENCY
				lat_start = rxF->regs[1];
			#endif

			fipc_recv_msg_end( forw, rxF );

			// Apply pipeline function to packet
			packet_ptr = pipe_func[rank]( (int64_t*)packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			#if  defined(FIPC_TEST_TIME_PER_TRANSACTION) || defined(FIPC_TEST_LATENCY)
				end = RDTSCP();
			#endif

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				times[transaction_id] = end - start;
			#endif

			#ifdef FIPC_TEST_LATENCY
				latencyTimes[transaction_id] = end - lat_start;
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

			// Receive message and store packet pointer locally
			fipc_test_blocking_recv_start( forw, &rxF );
			packet_ptr = (packet_t*) rxF->regs[0];

			#ifdef FIPC_TEST_LATENCY
				lat_start = rxF->regs[1];
			#endif

			fipc_recv_msg_end( forw, rxF );

			// Apply pipeline function to packet
			packet_ptr = pipe_func[rank]( (int64_t*)packet_ptr, MAX_LINES_USED*(FIPC_CACHE_LINE_SIZE/8) );

			// Send a pointer to the packet in a message to the next process
			fipc_test_blocking_send_start( forw, &txF );
			txF->regs[0] = (uint64_t) packet_ptr;

			#ifdef FIPC_TEST_LATENCY
				txF->regs[1] = lat_start;
			#endif

			fipc_send_msg_end ( forw, txF );

			#ifdef FIPC_TEST_TIME_PER_TRANSACTION
				end = RDTSCP();
				times[transaction_id] = end - start;
			#endif
		}
	}

	#ifndef FIPC_TEST_TIME_PER_TRANSACTION
		end = RDTSCP();
	#endif

	printf( "Hello4 from process %lu.\n", rank );
	///////////// Synchronized display of test metrics

	if ( rank == 0 )
	{
		#ifdef FIPC_TEST_TIME_PER_TRANSACTION
			printf( "Process %lu's stats", rank );
			fipc_test_stat_print_info( times, TRANSACTIONS );
		#endif

		fipc_test_blocking_send_start( forw, &txF );

		#ifndef FIPC_TEST_TIME_PER_TRANSACTION
			txF->regs[0] = start;
		#endif

		fipc_send_msg_end ( forw, txF );

	}
	else if ( rank == NUM_PROCESSORS-1 )
	{
		fipc_test_blocking_recv_start( forw, &rxF );

		#ifndef FIPC_TEST_TIME_PER_TRANSACTION
			start = rxF->regs[0];
		#endif

		fipc_recv_msg_end( forw, rxF );

		#ifndef FIPC_TEST_TIME_PER_TRANSACTION
			printf( "Time to send %lu messages through the pipeline: %lu\n", TRANSACTIONS, end - start - sync_offset );
			printf( "Time Sync Offset: %lu\n", sync_offset );
		#else
			printf( "Process %lu's stats", rank );
			fipc_test_stat_print_info( times, TRANSACTIONS );
		#endif

		#ifdef FIPC_TEST_LATENCY
			printf( "Latency Statistics" );
			fipc_test_stat_print_info( latencyTimes, TRANSACTIONS );
		#endif
	}
	else
	{
		fipc_test_blocking_recv_start( forw, &rxF );
		message_t temp = *rxF;
		fipc_recv_msg_end( forw, rxF );

		#ifdef FIPC_TEST_TIME_PER_TRANSACTION
			printf( "Process %lu's stats", rank );
			fipc_test_stat_print_info( times, TRANSACTIONS );
		#endif

		fipc_test_blocking_send_start( forw, &txF );
		*txF = temp;
		fipc_send_msg_end( forw, txF );
	}

	///////////// Synchronized cleanup

//	printf( "Hello from process %lu.\n", rank );

	//if ( rank != 3 )
	//	fipc_test_shm_unlink( shm_keys[rank] );

	//fipc_test_shm_free_half_channel( chan );

	fipc_fini();
	return 0;
}
