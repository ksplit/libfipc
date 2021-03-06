/*
 * caller.c
 *
 * The "caller side" of the channel
 *
 * Copyright: University of Utah
 */

#include <linux/random.h>
#include "empty_msg.h"
#include "../test_helpers.h"

static inline int do_one_msg(struct fipc_ring_channel *chan)
{
	int ret;
	struct fipc_message *request;
	struct fipc_message *response;
	/*
	 * Send empty message
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret)
		goto fail1;
	ret = fipc_send_msg_end(chan, request);
	if (ret)
		goto fail2;
	/*
	 * Receive empty response
	 */
	ret = test_fipc_blocking_recv_start(chan, &response);
	if (ret)
		goto fail3;
	ret = fipc_recv_msg_end(chan, response);
	if (ret)
		goto fail4;

	return 0;

fail4:
fail3:
fail2:
fail1:
	return ret;
}

int caller(void *_caller_channel_header)
{
        struct fipc_ring_channel *chan = _caller_channel_header;
	unsigned long transaction_id;
	unsigned long start, end;
	int ret = 0;
	/*
	 * Turn off interrupts so that we truly take over the core
	 */
	local_irq_disable();
	/*
	 * Do send/recv
	 */
	pr_err("Roundtrip Times (in cycles):\n");
	for (transaction_id = 0; 
	     transaction_id < TRANSACTIONS;
	     transaction_id++) {

		start = test_fipc_start_stopwatch();
		ret = do_one_msg(chan);
		end = test_fipc_stop_stopwatch();

		if (ret) {
			pr_err("error in send/recv, ret = %d, exiting...\n",
				ret);
			goto out;
		}

		pr_err("\t%lu\n", end - start);
	}

	pr_err("Complete\n");
	/*
	 * Re-enable interrupts
	 */
	local_irq_enable();

out:
	return ret;
}
