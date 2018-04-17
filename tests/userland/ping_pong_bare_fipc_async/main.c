/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 */

#include <thc.h>
#include <thcinternal.h>
#include <awe_mapper.h>

#include "thc_ipc_types.h"
#include "test.h"

//#define FINE_GRAINED
static uint64_t transactions   = 1000000;
static uint32_t num_inner_asyncs = 10; 

#define REQUESTER_CPU	1
#define RESPONDER_CPU	3
#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7
#define QUEUE_DEPTH	1

typedef enum {
	FOO = 2,
	FOO_BLOCKING = 3,
	DONE = 4,
} dispatch_t;

static int queue_depth = 1;

header_t* requester_header_xmit = NULL;
header_t* responder_header_xmit = NULL;

static inline
int
async_msg_get_fn_type(struct fipc_message *msg)
{
	return fipc_get_flags(msg) >> THC_RESERVED_MSG_FLAG_BITS;
}

static inline
void
async_msg_set_fn_type(struct fipc_message *msg, int type)
{
	uint32_t flags = fipc_get_flags(msg);
	/* ensure type is in range */
	type &= (1 << (32 - THC_RESERVED_MSG_FLAG_BITS)) - 1;
	/* erase old type */
	flags &= ((1 << THC_RESERVED_MSG_FLAG_BITS) - 1);
	/* install new type */
	flags |= (type << THC_RESERVED_MSG_FLAG_BITS);
	fipc_set_flags(msg, flags);
}



#if defined(FINE_GRAINED)
register uint64_t CACHE_ALIGNED start;
register uint64_t CACHE_ALIGNED end;
register int64_t* CACHE_ALIGNED times;
register uint64_t CACHE_ALIGNED correction;
#endif
uint64_t CACHE_ALIGNED whole_start;
uint64_t CACHE_ALIGNED whole_end;

void print_stats(int64_t *times, uint64_t transactions){

	// End test
#if defined(FINE_GRAINED) 
	register uint64_t CACHE_ALIGNED transaction_id;

	fipc_test_stat_get_and_print_stats( times, transactions );
	printf("Correction value: %llu\n", (unsigned long long) correction);
  	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		printf("%llu\n", (unsigned long long) times[transaction_id]);
	}

#endif
	// Print count
	printf("-------------------------------------------------\n");
	
}


void request ( header_t* chan )
{
	message_t* request;
	message_t* response;

	int i;

	for ( i = 0; i < queue_depth; ++i ) {
		fipc_test_blocking_send_start( chan, &request );
		fipc_send_msg_end ( chan, request );
	}

	for ( i = 0; i < queue_depth; ++i )
	{
		fipc_test_blocking_recv_start( chan, &response );
		fipc_recv_msg_end( chan, response );
	}
	
}

static inline
int fipc_test_blocking_recv_start_block ( header_t* channel, message_t** out, uint64_t id )
{
	int ret;
retry:
	while ( 1 )
	{
		// Poll until we get a message or error
		*out = get_current_rx_slot( channel);

		if ( ! check_rx_slot_msg_waiting( *out ) )
		{
			// No messages to receive, yield to next async
			//printf("No messages to recv, yield and save into id:%llu\n", id);
			THCYieldAndSave(id);
			continue; 
		}

		break;
	}

	if((*out)->regs[0] == id) {
		//printf("Message is ours id:%llu\n", (*out)->regs[0]);
		inc_rx_slot( channel ); 
		return 0;
	}
	
	//printf("Message is not ours yielding to id:%llu\n", (*out)->regs[0]);
	ret = THCYieldToIdAndSave((*out)->regs[0], id);
	 
	//ret = THCYieldToId((*out)->regs[0]);
	if (ret) {
		printf("ALERT: wrong id\n");
		return ret;
	}

	// We came back here but maybe we're the last AWE and 
        // we're re-started by do finish
	goto retry; 
	return 0;
}

void request_blk ( header_t* chan)
{
	message_t* req;
	message_t* resp;

	int id = awe_mapper_create_id();
	//printf("Got id:%d\n", id);

 	// Call
	fipc_test_blocking_send_start( chan, &req );
        req->regs[0] = (uint64_t)id;
	fipc_send_msg_end ( chan, req );

	// Reply (msg id is in reg[0]
	fipc_test_blocking_recv_start_block (chan, &resp, id);
	fipc_recv_msg_end( chan, resp );
	awe_mapper_remove_id(id);
	return; 	
}
void respond_ack ( header_t* chan )
{
	message_t* req;
	message_t* resp;

	uint64_t id;
	
	fipc_test_blocking_recv_start( chan, &req);
	id = req->regs[0];
	fipc_recv_msg_end( chan, req );
	
	fipc_test_blocking_send_start( chan, &resp );
	resp->regs[0] = id; 
	fipc_send_msg_end( chan, resp );
}

static inline unsigned long long load(){
	unsigned long long sum = 0;
	int i;

	for(i = 0; i < 100; i++)
		sum += i;

	return sum; 
}


void respond_ack_load100 ( header_t* chan )
{
	message_t* req;
	message_t* resp;
	uint64_t id;
	unsigned long long sum;
	
	fipc_test_blocking_recv_start( chan, &req);
	id = req->regs[0];
	fipc_recv_msg_end( chan, req );

	sum = load();

	fipc_test_blocking_send_start( chan, &resp );
	resp->regs[0] = id; 
	resp->regs[1] = sum;
	fipc_send_msg_end( chan, resp );
}


void respond ( header_t* chan )
{
	message_t* request;
	message_t* response;

	fipc_test_blocking_recv_start( chan, &request );
	fipc_recv_msg_end( chan, request );

	fipc_test_blocking_send_start( chan, &response );
	fipc_send_msg_end( chan, response );
}


void ping_pong_rsp(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		respond( chan );
	}
}

void ping_pong_rsp_load100(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		respond_ack_load100( chan );
	}
}

void no_async_10_rsp(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j; 
		for (j = 0; j < num_inner_asyncs; j++) {
			respond( chan );
		}
	}
}

void async_10_rsp(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j; 
		for (j = 0; j < num_inner_asyncs; j++) {
			respond_ack( chan );
		}
	}
}

void async_10_rsp_blk(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j; 
		for (j = 0; j < num_inner_asyncs; j++) {
			respond_ack( chan );
		}
	}
}

void async_10_rsp_blk_load100(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j; 
		for (j = 0; j < num_inner_asyncs; j++) {
			respond_ack_load100( chan );
		}
	}
}


void async_10_rsp_blk_srv_async_dispatch(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j; 
		DO_FINISH({
			for (j = 0; j < num_inner_asyncs; j++) {
				ASYNC({
					respond_ack( chan );
				});
			}
		});
	}
}

void async_10_rsp_blk_srv_async_dispatch_load100(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j; 
		DO_FINISH({
			for (j = 0; j < num_inner_asyncs; j++) {
				ASYNC({
					respond_ack_load100( chan );
				});
			}
		});
	}
}

void check_load(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;
	unsigned long long sum; 

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif
		sum = load();

#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();
#if defined(FINE_GRAINED)		
 	print_stats(times, transactions);
#endif
 	printf("%llu (sum = %llu)\n",  
			(unsigned long long) (whole_end - whole_start) / transactions, sum);

	return;

}

void ping_pong_req(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif
		request( chan );
#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();
#if defined(FINE_GRAINED)		
 	print_stats(times, transactions);
#endif
 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);


	pthread_mutex_unlock( &requester_mutex );
	return;

}

void no_async_10_req(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j;
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif
		DO_FINISH({
			for (j = 0; j < num_inner_asyncs; j++) {
				request( chan );
			};
		});
#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();
#if defined(FINE_GRAINED)		
 	print_stats(times, transactions);
#endif
 	printf(" %llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);


	pthread_mutex_unlock( &requester_mutex );
	return;

}

void async_10_req(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j;
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif
		DO_FINISH({
			for (j = 0; j < num_inner_asyncs; j++) {
				ASYNC({
					request( chan );
				});
			};
		});
#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();
#if defined(FINE_GRAINED)		
 	print_stats(times, transactions);
#endif
 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);


	pthread_mutex_unlock( &requester_mutex );
	return;

}

void async_10_req_blk(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j;
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif
		DO_FINISH({
			for (j = 0; j < num_inner_asyncs; j++) {
				ASYNC({
					request_blk( chan );
				});
			};
		});
#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();
#if defined(FINE_GRAINED)		
 	print_stats(times, transactions);
#endif
 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);


	pthread_mutex_unlock( &requester_mutex );
	return;

}

void async_10_req_blk_srv_async_dispatch(header_t *chan) {
	register uint64_t CACHE_ALIGNED transaction_id;

	// Wait to begin test
	pthread_mutex_lock( &requester_mutex );

	whole_start = RDTSC_START();

	// Begin test
	for ( transaction_id = 0; transaction_id < transactions; transaction_id++ )
	{
		int j;
#if defined(FINE_GRAINED)		
		start = RDTSC_START();
#endif
		DO_FINISH({
			for (j = 0; j < num_inner_asyncs; j++) {
				ASYNC({
					request_blk( chan );
				});
			};
		});
#if defined(FINE_GRAINED)		
		end = RDTSCP();
		times[transaction_id] = (end - start) - correction;
#endif		
	}

	whole_end = RDTSCP();
#if defined(FINE_GRAINED)		
 	print_stats(times, transactions);
#endif
 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);


	pthread_mutex_unlock( &requester_mutex );
	return;

}

//assumes msg is a valid received message
static int poll_recv_predicate(message_t* msg, void* data)
{
    struct predicate_payload* payload_ptr = (struct predicate_payload*)data;

    if( likely(thc_get_msg_type(msg) == (uint32_t)msg_type_request ))
    {   
        payload_ptr->msg_type = msg_type_request;
//	printf("%s, got request\n", __func__);
        return 1;
    }   
    else
    {   
        payload_ptr->actual_msg_id = thc_get_msg_id(msg);
        return 0; //message not for this awe
    }   
}

static inline
int fipc_test_blocking_recv_start_if ( header_t* channel, message_t** out)
{
retry:
	while ( 1 )
	{
		// Poll until we get a message or error
		*out = get_current_rx_slot( channel);

		if ( ! check_rx_slot_msg_waiting( *out ) )
		{
			//printf("No messages to recv, yield and save into id:%llu\n", id);
			continue; 
		}

		break;
	}

	if( likely(thc_get_msg_type(*out) == (uint32_t)msg_type_request ))
	{   
//		printf("%s:%d got request\n", __func__, __LINE__);
		inc_rx_slot (channel);
 		return 0;
	}
	else
	{   
		printf("%s:%d msg not for us!\n", __func__, __LINE__);
		return 1; //message not for this awe
	}

	//printf("Message is not ours yielding to id:%llu\n", (*out)->regs[0]);
//	ret = THCYieldToIdAndSave((*out)->regs[0], id);
	 
	//ret = THCYieldToId((*out)->regs[0]);
//	if (ret) {
//		printf("ALERT: wrong id\n");
//		return ret;
//	}

	// We came back here but maybe we're the last AWE and 
        // we're re-started by do finish
	goto retry; 
	return 0;
}

int thc_ipc_poll_recv(header_t *rx_chan, message_t **out_msg)
{
	struct predicate_payload payload;
	int ret;

	while (1) {
		ret = fipc_recv_msg_if(rx_chan, poll_recv_predicate, &payload, out_msg);
		if (likely(!ret)) {
//		printf("%s, got %d\n", __func__, ret);
			return 0;
		} else if (ret == -ENOMSG) {
			THCYieldToId((uint32_t)payload.actual_msg_id);
		} else if (ret == -EWOULDBLOCK) {
			return ret;
		} else {
			printf("%s:%d error\n", __func__, __LINE__);
			return ret;
		}
	}
}

static inline header_t *
thc_channel_group_item_channel(struct thc_channel_group_item *item)
{
    return item->channel;
}


int 
thc_poll_recv_group(struct thc_channel_group* chan_group, 
		struct thc_channel_group_item** chan_group_item, 
		message_t** out_msg)
{
    struct thc_channel_group_item *curr_item;
    message_t* recv_msg;
    int ret;

    list_for_each_entry(curr_item, &(chan_group->head), list)
    {
        ret = thc_ipc_poll_recv(thc_channel_group_item_channel(curr_item), 
                        &recv_msg);
        if( !ret )
        {
            *chan_group_item = curr_item;
            *out_msg         = recv_msg;
            
            return 0;
        }
    }

    return -EWOULDBLOCK;
}

int foo_callee(message_t *req, header_t *chan)
{
	unsigned int request_cookie;
	message_t *resp;

	request_cookie = thc_get_request_cookie(req);

	fipc_recv_msg_end(chan, req);

	fipc_send_msg_start(chan, &resp);

	thc_set_msg_type(resp, msg_type_response);
	thc_set_msg_id(resp, request_cookie);

	fipc_send_msg_end(chan, resp);
	return 0;
}

int foo_blocking_callee(message_t *req, header_t *chan)
{
	message_t *resp;

	fipc_recv_msg_end(chan, req);

	fipc_test_blocking_send_start( chan, &resp );
	fipc_send_msg_end(chan, resp);
	return 0;
}

int respond_dispatch_async_loop(header_t *chan, message_t *msg)
{
	int fn_type = async_msg_get_fn_type(msg);
	switch (fn_type) {
	case FOO:
//		printf("%s, dispatching foo %d\n", __func__, iter);
		return foo_callee(msg, chan);
	case FOO_BLOCKING:
//		printf("%s, dispatching foo_blocking\n", __func__);
		return foo_blocking_callee(msg, chan);
	case DONE:
		fipc_recv_msg_end(chan, msg);
		return DONE;
	default:
		return 1;
	}
}

int __respond_dispatch(header_t *chan)
{
	message_t *msg;
	int stop = 0;
	int ret;

//	DO_FINISH({
	while (!stop) {
		ret = thc_ipc_poll_recv(chan, &msg);

		if (ret) {
			if (ret == -EWOULDBLOCK) {
				continue;
			} else {
				stop = 1;
				printf("%s:%d error %d\n", __func__, __LINE__, ret);
			}
		}
//		printf("%s, dispatching %d\n", __func__, thc_get_msg_type(msg));
//		ASYNC(
		ret = respond_dispatch_async_loop(chan, msg);
//		);

		if (ret)
			return ret;	
	}
//	});
	return ret;
}

struct thc_channel_group ch_grp;

int respond_dispatch_two_channels(header_t *chan1, header_t *chan2)
{
	message_t *msg;
	int stop = 0;
	int ret;

	struct thc_channel_group_item *xmit_ch_item;
	struct thc_channel_group_item *item_ch1;

	thc_channel_group_init(&ch_grp);

	xmit_ch_item = malloc(sizeof(*xmit_ch_item));
	item_ch1 = malloc(sizeof(*item_ch1));

	thc_channel_group_item_init(xmit_ch_item, chan1, NULL);

	thc_channel_group_item_init(item_ch1, chan2, NULL);

	thc_channel_group_item_add(&ch_grp, xmit_ch_item);
	thc_channel_group_item_add(&ch_grp, item_ch1);

	DO_FINISH(
	while (!stop) {
		struct thc_channel_group_item* curr_item;

		ret = thc_poll_recv_group(&ch_grp, &curr_item, &msg);

		if (ret) {
			if (ret == -EWOULDBLOCK) {
				continue;
			} else {
				stop = 1;
				printf("%s:%d error %d\n", __func__, __LINE__, ret);
			}
		}
		ASYNC(
		ret = respond_dispatch_async_loop(
				curr_item->channel,
				msg);
		if (ret) {
			printf("%s:%d dispatch loop returned %d\n",
					__func__, __LINE__, ret);
			stop = 1;
		}
		);
	}
	);
	return ret;
}

int respond_dispatch(header_t *chan)
{
	message_t *msg;
	int stop = 0;
	int ret;

//	printf("%s, looping\n", __func__);
	while (!stop) {
		ret = thc_ipc_poll_recv(chan, &msg);

		if (ret) {
			if (ret == -EWOULDBLOCK) {
				continue;
			} else {
				stop = 1;
				printf("%s:%d error %d\n", __func__, __LINE__, ret);
			}
		}
		ret = respond_dispatch_async_loop(chan, msg);
		if (ret)
			return ret;
	}
	return ret;
}

int respond_dispatch_2(header_t *chan)
{
	message_t *msg;
	int stop = 0;
	int ret;

	while (!stop) {
		ret = fipc_test_blocking_recv_start_if(chan, &msg);

		if (ret) {
			if (ret == -EWOULDBLOCK) {
				continue;
			} else {
				stop = 1;
				printf("%s:%d error %d\n", __func__, __LINE__, ret);
			}
		} else {
			ret = respond_dispatch_async_loop(chan, msg);
			if (ret)
				return ret;
		}
	}
	return ret;
}


int
thc_ipc_send_request(header_t *chnl,
		message_t *request,
		uint32_t *request_cookie)
{
    uint32_t msg_id;
    int ret = 0;
    /*
     * Get an id for our current awe, and store in request.
     */
    msg_id = awe_mapper_create_id();

    if (ret) {
        printf("thc_ipc_send: error getting request cookie\n");
	goto fail0;
    }
    thc_set_msg_type(request, msg_type_request);
    thc_set_msg_id(request, msg_id);
    /*
     * Send request
     */
    ret = fipc_send_msg_end(chnl, request);
    if (ret) {
        printf("thc: error sending request");
        goto fail1;	
    }

    *request_cookie = msg_id;

    return 0;

fail1:
    awe_mapper_remove_id(msg_id);
fail0:
    return ret;
}

static int thc_recv_predicate(struct fipc_message* msg, void* data)
{
	struct predicate_payload* payload_ptr = (struct predicate_payload*)data;
	payload_ptr->msg_type = thc_get_msg_type(msg);

	if (unlikely(payload_ptr->msg_type == msg_type_request)) {
		/*
		 * Ignore requests
		 */
		return 0;
	} else if (payload_ptr->msg_type == msg_type_response) {

		payload_ptr->actual_msg_id = thc_get_msg_id(msg);

		if (payload_ptr->actual_msg_id == 
			payload_ptr->expected_msg_id) {
			/*
			 * Response is for us; tell libfipc we want it
			 */
			return 1;
		} else {
			/*
			 * Response for another awe; we will switch to
			 * them. Meanwhile, the message should stay in
			 * rx. They (the awe we switch to) will pick it up.
			 */
			return 0;
		}
	} else {
		/*
		 * Ignore any other message types
		 */
		return 0;
	}
}

static void drop_one_rx_msg(header_t *chnl)
{
	int ret;
	struct fipc_message *msg;

	ret = fipc_recv_msg_start((chnl), &msg);
	if (ret)
		printf( "thc_ipc_recv_response: failed to drop bad message\n");
	ret = fipc_recv_msg_end((chnl), msg);
	if (ret)
		printf( "thc_ipc_recv_response: failed to drop bad message (mark as received)\n");
	return;
}

static void try_yield(header_t *chnl, uint32_t our_request_cookie,
		uint32_t received_request_cookie)
{
	int ret;
	/*
	 * Switch to the pending awe the response belongs to
	 */
	ret = THCYieldToIdAndSave(received_request_cookie,
				our_request_cookie);
	if (ret) {
		/*
		 * Oops, the switch failed
		 */
		printf( "thc_ipc_recv_response: Invalid request cookie 0x%x received; dropping the message\n",
			received_request_cookie);
		drop_one_rx_msg(chnl);
		return;
	}
	/*
	 * We were woken back up
	 */
	return;
}

int 
thc_ipc_recv_response(header_t *chnl, 
		uint32_t request_cookie, 
		message_t **response)
{
	struct predicate_payload payload = {
		.expected_msg_id = request_cookie
	};
	int ret;

retry:
        ret = fipc_recv_msg_if(chnl, thc_recv_predicate, 
			&payload, response);
	if (likely(ret == 0)) {
		/*
		 * Message for us; remove request_cookie from awe mapper
		 */
		//printf("%s:%d message for us, removing %d!\n", __func__, __LINE__, request_cookie);
                awe_mapper_remove_id(request_cookie);
		return 0;
	} else if (ret == -ENOMSG && payload.msg_type == msg_type_request) {
		/*
		 * Ignore requests; yield so someone else can receive it (msgs
		 * are received in fifo order).
		 */
		goto yield;
	} else if (ret == -ENOMSG && payload.msg_type == msg_type_response) {
		/*
		 * Response for someone else. Try to yield to them.
		 */
		try_yield(chnl, request_cookie, payload.actual_msg_id);
		/*
		 * We either yielded to the pending awe the response
		 * belonged to, or the switch failed.
		 *
		 * Make sure the channel didn't die in case we did go to
		 * sleep.
		 */
		goto retry;
	} else if (ret == -ENOMSG) {
		/*
		 * Unknown or unspecified message type; yield and let someone
		 * else handle it.
		 */
		goto yield;
	} else if (ret == -EWOULDBLOCK) {
		/*
		 * No messages in rx buffer; go to sleep.
		 */
		//printf("%s:%d, no message -> yield\n", __func__, __LINE__);
		goto yield;
	} else {
		/*
		 * Error
		 */
		printf( "thc_ipc_recv_response: fipc returned %d\n", 
			ret);
		return ret;
	}

yield:
	/*
	 * Go to sleep, we will be woken up at some later time
	 * by the dispatch loop or some other awe.
	 */
	THCYieldAndSave(request_cookie);
//	printf("%s:%d woken up %d\n", __func__, __LINE__, request_cookie);
	/*
	 * We were woken up; make sure the channel didn't die while
	 * we were asleep.
	 */
	goto retry;
}

static inline
int thc_ipc_recv_response_new ( header_t* channel, message_t** out, uint64_t id )
{
	int ret;
	int received_cookie;
retry:
	while ( 1 )
	{
		// Poll until we get a message or error
		*out = get_current_rx_slot( channel);

		if ( ! check_rx_slot_msg_waiting( *out ) )
		{
			// No messages to receive, yield to next async
			//printf("No messages to recv, yield and save into id:%llu\n", id);
			THCYieldAndSave(id);
			continue; 
		}

		break;
	}

	received_cookie = thc_get_msg_id(*out);
	if (received_cookie == id) {
		//printf("Message is ours id:%llu\n", (*out)->regs[0]);
		inc_rx_slot( channel ); 
		return 0;
	}
	
	//printf("Message is not ours yielding to id:%llu\n", (*out)->regs[0]);
	ret = THCYieldToIdAndSave(received_cookie, id);
	 
	//ret = THCYieldToId((*out)->regs[0]);
	if (ret) {
		printf("ALERT: wrong id\n");
		return ret;
	}

	// We came back here but maybe we're the last AWE and 
        // we're re-started by do finish
	goto retry; 
	return 0;
}


int foo(header_t *chan)
{
	message_t *req, *resp;
	unsigned int request_cookie;
	int ret;

	fipc_send_msg_start(chan, &req);

	async_msg_set_fn_type(req, FOO);

	ret = thc_ipc_send_request(chan, req, &request_cookie);

//	printf("%s, sending reqcookie %d\n", __func__, request_cookie);
	if (ret)
		printf("%s:%d send error\n", __func__, __LINE__);


//	ret = thc_ipc_recv_response(chan, request_cookie, &resp); 
	ret = thc_ipc_recv_response_new(chan, &resp, request_cookie);

	awe_mapper_remove_id(request_cookie);

	if (ret)
		printf("%s:%d send error\n", __func__, __LINE__);

	fipc_recv_msg_end( chan, resp );

//	printf("%s, got response for request_cookie %d\n", __func__, request_cookie);

	return ret;
}

int foo_blocking(header_t *chan)
{
	message_t *req, *resp;

	fipc_send_msg_start(chan, &req);

	/* for dispatching the request at the responder */
	async_msg_set_fn_type(req, FOO_BLOCKING);
    	thc_set_msg_type(req, msg_type_request);

	fipc_send_msg_end ( chan, req );

	fipc_test_blocking_recv_start(chan, &resp);

	fipc_recv_msg_end(chan, resp); 

	return 0;
}

/*
 * to signal that the sender is done so that the receiver can exit the dispatch loop
 */
void done(header_t *chan)
{
	message_t *req;
	unsigned int request_cookie;
	int ret;

	fipc_send_msg_start(chan, &req);

	async_msg_set_fn_type(req, DONE);

	ret = thc_ipc_send_request(chan, req, &request_cookie);

	if (ret)
		printf("%s:%d send error\n", __func__, __LINE__);
	awe_mapper_remove_id(request_cookie);
//	printf("%s:%d sending done\n", __func__, __LINE__);
}

int request_dispatch_blocking(header_t *chan)
{
	int i;
	whole_start = RDTSC_START();

	for (i = 0; i < transactions; i++) {
		foo_blocking(chan);
	}

	whole_end = RDTSCP();

 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);
	done(chan);
	return 0;
}

int request_dispatch_async_send(header_t *chan)
{
	int i;
	whole_start = RDTSC_START();


	for (i = 0; i < transactions; i++) {
	//	printf("%s:%d sending %d\n", __func__, __LINE__, i);
		DO_FINISH(
			ASYNC(foo(chan));
		);
	}

	whole_end = RDTSCP();

 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);
	done(chan);
	return 0;
}

int request_dispatch_10_async_send(header_t *chan)
{
	int i;
	whole_start = RDTSC_START();
	int t;

	for (i = 0; i < transactions; i++) {
	//	printf("%s:%d sending %d\n", __func__, __LINE__, i);
		DO_FINISH(
			for (t = 0; t < num_inner_asyncs; t++)
				ASYNC(foo(chan));
		);
	}

	whole_end = RDTSCP();

 	printf("%llu\n",  
			(unsigned long long) (whole_end - whole_start) / transactions);
	done(chan);
	return 0;
}

void* requester ( void* data )
{
	header_t* chan = (header_t*) data;
#if defined(FINE_GRAINED)		
	register int64_t* CACHE_ALIGNED times = malloc( transactions * sizeof( int64_t ) );
	correction = fipc_test_time_get_correction();
#endif
	thc_init();
#if 1
	printf("load100 function:");
	check_load(chan);

	printf("ping-pong 1 msg:"); 
	ping_pong_req(chan);
 	printf("ping-pong 1 msg, load 100:"); 
	ping_pong_req(chan);

	printf("do_finish (10 msgs):");
	no_async_10_req(chan);
	printf("do_finish (10 msgs), load 100:");
	no_async_10_req(chan);

	printf("do{async{}}finish(), 10 msgs:");
	async_10_req(chan);
	printf("do{async{}}finish(), 10 msgs, load 100:");
	async_10_req(chan);

	printf("do{async{send and yield}}finish(), 10 msgs:"); 
	async_10_req_blk(chan);
	printf("do{async{send and yeild}}finish(), 10 msgs, load 100:"); 
	async_10_req_blk(chan);


	printf("do{async{send_blk}}finish(), 10 msgs, srv async dispatch:"); 
        async_10_req_blk_srv_async_dispatch(chan);

	printf("do{async{send_blk}}finish(), 10 msgs, srv async dispatch, load 100:");
	async_10_req_blk_srv_async_dispatch(chan);
#endif
	printf("blocking sender, dispatch loop on the responder:");
	request_dispatch_blocking(chan);

	printf("async send, dispatch loop on the responder:");
	request_dispatch_async_send(chan);

	printf("do{async{}}finish(): 10 msgs: dispatch loop on the responder:");
	request_dispatch_10_async_send(chan);

	printf("private_channel: recv dispatch loop on the channel group:");
	request_dispatch_10_async_send(requester_header_xmit);

	thc_done();

#if defined(FINE_GRAINED)		
	free( times );
#endif
	pthread_exit( 0 );

	return NULL;
}

void* responder ( void* data )
{
	header_t* chan = (header_t*) data;
	
	thc_init();

#if 1
	// Wait to begin test
	pthread_mutex_lock( &responder_mutex );

	// ping-pong 1 msg
	ping_pong_rsp(chan);
	// ping-pong 1 msg, load 100
	ping_pong_rsp_load100(chan); 
	
	// do_finish (10 msgs)
	async_10_rsp(chan);
	// do_finish (10 msgs), load 100
	async_10_rsp_blk_load100(chan); 
        // do{async{}}finish(), 10 msgs	
	async_10_rsp(chan); 

	// do{async{}}finish(), 10 msgs, load 100
	async_10_rsp_blk_load100(chan);

	// do{async{send and yield}}finish(), 10 msgs	
	async_10_rsp_blk(chan);
	// do{async{send and yield}}finish(), 10 msgs, load 100	
	async_10_rsp_blk_load100(chan);

	// do{async{send_blk}}finish(), 10 msgs, srv async dispatch
	async_10_rsp_blk_srv_async_dispatch(chan);
	// do{async{send_blk}}finish(), 10 msgs, srv async dispatch, load 100
	async_10_rsp_blk_srv_async_dispatch_load100(chan);
	// End test
#endif	
	// blocking send, dispatch loop at receiver
	respond_dispatch(chan);

	// async send, dispatch loop at receiver
	respond_dispatch(chan);

	// 10 async send, dispatch loop at receiver
	respond_dispatch(chan);

	// 2 channels
	respond_dispatch_two_channels(chan, responder_header_xmit);

	pthread_mutex_unlock( &responder_mutex );
	thc_done();
	pthread_exit( 0 );
	return NULL;
}


int main ( void )
{
	// Begin critical section
	pthread_mutex_init( &requester_mutex, NULL );
	pthread_mutex_init( &responder_mutex, NULL );
	
	pthread_mutex_lock( &requester_mutex );
	pthread_mutex_lock( &responder_mutex );
	
	header_t* requester_header = NULL;
	header_t* responder_header = NULL;

	fipc_init();
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header, &responder_header );
	fipc_test_create_channel( CHANNEL_ORDER, &requester_header_xmit, &responder_header_xmit );

	if ( requester_header == NULL || responder_header == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating channel" );
		return -1;
	}
	if ( requester_header_xmit == NULL || responder_header_xmit == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating channel" );
		return -1;
	}

	// Create Threads
	pthread_t* requester_thread = fipc_test_thread_spawn_on_CPU ( requester, requester_header, REQUESTER_CPU );
	if ( requester_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	pthread_t* responder_thread = fipc_test_thread_spawn_on_CPU ( responder, responder_header, RESPONDER_CPU );
	if ( responder_thread == NULL )
	{
		fprintf( stderr, "%s\n", "Error while creating thread" );
		return -1;
	}
	
	// End critical section = start threads
	pthread_mutex_unlock( &responder_mutex );
	pthread_mutex_unlock( &requester_mutex );
	
	// Wait for thread completion
	fipc_test_thread_wait_for_thread( requester_thread );
	fipc_test_thread_wait_for_thread( responder_thread );
	
	// Clean up
	fipc_test_thread_free_thread( requester_thread );
	fipc_test_thread_free_thread( responder_thread );
	fipc_test_free_channel( CHANNEL_ORDER, requester_header, responder_header );
	fipc_fini();
	pthread_exit( NULL );
	return 0;
}
