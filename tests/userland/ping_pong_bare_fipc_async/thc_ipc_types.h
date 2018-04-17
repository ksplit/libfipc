#ifndef THC_IPC_TYPES_H
#define THC_IPC_TYPES_H

#include <libfipc.h>
#include "list.h"

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

/*
 * struct thc_channel_group_item
 *
 * Contains a channel and a function that should get called when a message
 * is received on the channel.
 */
struct thc_channel_group_item
{
    struct list_head list;
    header_t *channel;
    int (*dispatch_fn)(struct thc_channel_group_item*, message_t *);
};

/*
 * struct thc_channel_group
 *
 * Represents a linked list of thc_channel_group_items.
 */
struct thc_channel_group
{
    struct list_head head;
    int size;
};

int 
thc_channel_group_init(struct thc_channel_group* channel_group)
{
    INIT_LIST_HEAD(&(channel_group->head));
    channel_group->size = 0;

    return 0;
}

int
thc_channel_group_item_init(struct thc_channel_group_item *item,
			header_t *chnl,
			int (*dispatch_fn)(struct thc_channel_group_item*, 
					struct fipc_message*))
{
    INIT_LIST_HEAD(&item->list);
    item->channel = chnl;
    item->dispatch_fn = dispatch_fn;

    return 0;
}

int 
thc_channel_group_item_add(struct thc_channel_group* channel_group, 
                          struct thc_channel_group_item* item)
{
    list_add_tail(&(item->list), &(channel_group->head));
    channel_group->size++;

    return 0;
}

void
thc_channel_group_item_remove(struct thc_channel_group* channel_group, 
			struct thc_channel_group_item* item)
{
    list_del_init(&(item->list));
    channel_group->size--;
}

int
thc_channel_group_item_get(struct thc_channel_group* channel_group, 
                               int index, 
                               struct thc_channel_group_item **out_item)
{

    int curr_index = 0;
    list_for_each_entry((*out_item), &(channel_group->head), list)
    {
        if( curr_index == index )
        {
            return 0;
        }
        curr_index++;
    }

    return 1;
}

#endif
