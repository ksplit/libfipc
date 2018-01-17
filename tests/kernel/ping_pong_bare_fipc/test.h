/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test sends an (many) empty message(s) in a round trip
 * using the fipc library.
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 12

// Test Variables
static uint32_t transactions   = 1000000;
static uint8_t  requester_cpu  = 0;
static uint8_t  responder_cpu  = 4;
static uint32_t queue_depth    = 1;

module_param( transactions,     uint, 0 );
module_param( requester_cpu,    byte, 0 );
module_param( responder_cpu,    byte, 0 );
module_param( queue_depth,      uint, 0 );

// Thread Locks
struct completion requester_comp;
struct completion responder_comp;

#endif
