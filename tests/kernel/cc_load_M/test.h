/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This tests times a load from modified cache transaction
 *
 * This test uses a shared-continuous region of memory
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_CC_LOAD_M
#define LIBFIPC_TEST_CC_LOAD_M

#include "../libfipc_test.h"

// Test Variables
static uint32_t transactions   = 250;
static uint8_t  loader_cpu     = 0;
static uint8_t  stager_cpu     = 1;

module_param( transactions,     uint, 0 );
module_param( loader_cpu,       byte, 0 );
module_param( stager_cpu,       byte, 0 );

// Thread Locks
struct completion loader_comp;
struct completion stager_comp;

// Cache Variable
volatile cache_line_t* cache;

#define MSG_READY 0xF00DF00D

#endif
