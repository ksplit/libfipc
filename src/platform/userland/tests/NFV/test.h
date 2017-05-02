/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This test passes a simulated packet through a series of processes, which
 * represent composed functions. This is done using separate address spaces
 * to isolate the functions.
 *
 * NOTE: This test assumes an x86 architecture.
 *
 * NOTE: This test assumes a computer with 2-32 processing units.
 */

#include "../libfipc_test.h"

//#define FIPC_TEST_LATENCY
//#define FIPC_TEST_TIME_PER_TRANSACTION

#define CHANNEL_ORDER  ilog2(sizeof(message_t)) + 7
#define TRANSACTIONS   1000000LU
#define MAX_LINES_USED 8
#define NUM_PROCESSORS 3
#define CPU_OPERATIONS 1000LU

#include "functions.h"

const char* shm_keysF[] =
{
	"FIPC_NFV_S0_F",
	"FIPC_NFV_S1_F",
	"FIPC_NFV_S2_F",
	"FIPC_NFV_S3_F",
	"FIPC_NFV_S4_F",
	"FIPC_NFV_S5_F",
	"FIPC_NFV_S6_F",
	"FIPC_NFV_S7_F",
	"FIPC_NFV_S8_F",
	"FIPC_NFV_S9_F",
	"FIPC_NFV_S10_F",
	"FIPC_NFV_S11_F",
	"FIPC_NFV_S12_F",
	"FIPC_NFV_S13_F",
	"FIPC_NFV_S14_F",
	"FIPC_NFV_S15_F",
	"FIPC_NFV_S16_F",
	"FIPC_NFV_S17_F",
	"FIPC_NFV_S18_F",
	"FIPC_NFV_S19_F",
	"FIPC_NFV_S20_F",
	"FIPC_NFV_S21_F",
	"FIPC_NFV_S22_F",
	"FIPC_NFV_S23_F",
	"FIPC_NFV_S24_F",
	"FIPC_NFV_S25_F",
	"FIPC_NFV_S26_F",
	"FIPC_NFV_S27_F",
	"FIPC_NFV_S28_F",
	"FIPC_NFV_S29_F",
	"FIPC_NFV_S30_F"
};

const char* shm_keysB[] =
{
	"FIPC_NFV_S0_B",
	"FIPC_NFV_S1_B",
	"FIPC_NFV_S2_B",
	"FIPC_NFV_S3_B",
	"FIPC_NFV_S4_B",
	"FIPC_NFV_S5_B",
	"FIPC_NFV_S6_B",
	"FIPC_NFV_S7_B",
	"FIPC_NFV_S8_B",
	"FIPC_NFV_S9_B",
	"FIPC_NFV_S10_B",
	"FIPC_NFV_S11_B",
	"FIPC_NFV_S12_B",
	"FIPC_NFV_S13_B",
	"FIPC_NFV_S14_B",
	"FIPC_NFV_S15_B",
	"FIPC_NFV_S16_B",
	"FIPC_NFV_S17_B",
	"FIPC_NFV_S18_B",
	"FIPC_NFV_S19_B",
	"FIPC_NFV_S20_B",
	"FIPC_NFV_S21_B",
	"FIPC_NFV_S22_B",
	"FIPC_NFV_S23_B",
	"FIPC_NFV_S24_B",
	"FIPC_NFV_S25_B",
	"FIPC_NFV_S26_B",
	"FIPC_NFV_S27_B",
	"FIPC_NFV_S28_B",
	"FIPC_NFV_S29_B",
	"FIPC_NFV_S30_B"
};

void* (* const pipe_func[])(void*, const uint64_t) =
{
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum,
	CPU_intense,
	Fletcher_checksum
};

typedef struct packet
{
	cache_line_t data[MAX_LINES_USED] CACHE_ALIGNED;

} packet_t;
