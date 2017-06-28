/*
 * callee.c
 *
 * Code for the "callee side" of the channel
 *
 * Copyright: University of Utah
 */

#include <linux/kernel.h>
#include <libfipc.h>
#include "empty_msg.h"
#include "../test_helpers.h"

static inline int do_one_msg(struct fipc_ring_channel *chan)
{
	int ret;
	struct fipc_message *request;
	struct fipc_message *response;
	/*
	 * Receive empty request
	 */
	ret = test_fipc_blocking_recv_start(chan, &request);
	if (ret)
		goto fail1;
	ret = fipc_recv_msg_end(chan, request);
	if (ret)
		goto fail2;
	/*
	 * Send empty response
	 */
	ret = test_fipc_blocking_send_start(chan, &response);
	if (ret)
		goto fail3;
	ret = fipc_send_msg_end(chan, response);
	if (ret)
		goto fail4;

	return 0;

fail4:
fail3:
fail2:
fail1:
	return ret;
}

int callee(void *_callee_channel_header)
{
        struct fipc_ring_channel *chan = _callee_channel_header;
	unsigned long transaction_id;
	int ret = 0;
	/*
	 * Turn off interrupts so that we truly take over the core
	 */
	local_irq_disable();
	/*
	 * Do recv/send
	 */
	for (transaction_id = 0; 
	     transaction_id < TRANSACTIONS;
	     transaction_id++) {

		ret = do_one_msg(chan);

		if (ret) {
			pr_err("error in send/recv, ret = %d, exiting...\n",
				ret);
			goto out;
		}

	}
	/*
	 * Re-enable interrupts
	 */
	local_irq_enable();

out:
	return ret;
}
