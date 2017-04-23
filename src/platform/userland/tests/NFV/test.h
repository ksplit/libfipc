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
 * NOTE: This test assumes a computer with 2-16 processing units.
 */

#include "../libfipc_test.h"
#include "functions.h"

#define CHANNEL_ORDER  ilog2(sizeof(message_t)) + 7
#define TRANSACTIONS   100000LU
#define MAX_LINES_USED 8
#define NUM_PROCESSORS 3

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
	"FIPC_NFV_S14_F"
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
	"FIPC_NFV_S14_B"
};

void* (* const pipe_func[])(void*, const uint64_t) =
{
	XOR_checksum,
	SUM_checksum,
	MUL_checksum,
	Fletcher_checksum,
	XOR_checksum,
	SUM_checksum,
	MUL_checksum,
	Fletcher_checksum,
	XOR_checksum,
	SUM_checksum,
	MUL_checksum,
	Fletcher_checksum,
	XOR_checksum,
	SUM_checksum,
	MUL_checksum,
	Fletcher_checksum
};

typedef struct packet
{
	cache_line_t data[MAX_LINES_USED] CACHE_ALIGNED;

} packet_t;
