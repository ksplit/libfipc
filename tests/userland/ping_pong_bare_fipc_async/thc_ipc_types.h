#ifndef THC_IPC_TYPES_H
#define THC_IPC_TYPES_H

#include <libfipc.h>

typedef enum {
    msg_type_unspecified,
    msg_type_request,
    msg_type_response,
} msg_type_t;

struct predicate_payload
{
    uint32_t expected_msg_id;
    uint32_t actual_msg_id;
    msg_type_t msg_type;
};


enum {
    THC_CHANNEL_LIVE,
    THC_CHANNEL_DEAD,
};

#endif
