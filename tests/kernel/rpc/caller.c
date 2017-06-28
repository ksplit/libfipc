/*
 * caller.c
 *
 * The "caller side" of the channel
 *
 * Copyright: University of Utah
 */

#include <linux/random.h>
#include "rpc.h"
#include "../test_helpers.h"

static inline int send_and_get_response(
	struct fipc_ring_channel *chan,
	struct fipc_message *request,
	struct fipc_message **response)
{
	int ret;
	struct fipc_message *resp;
	/*
	 * Mark the request as sent
	 */
	ret = fipc_send_msg_end(chan, request);
	if (ret) {
		pr_err("failed to mark request as sent, ret = %d\n", ret);
		goto fail1;
	}
	/*
	 * Try to get the response
	 */
	ret = test_fipc_blocking_recv_start(chan, &resp);
	if (ret) {
		pr_err("failed to get a response, ret = %d\n", ret);
		goto fail2;
	}
	*response = resp;

	return 0;

fail2:
fail1:
	return ret;
}

#ifdef CHECK_MESSAGES

static inline int finish_response_check_fn_type(struct fipc_ring_channel *chnl,
						struct fipc_message *response,
						enum fn_type expected_type)
{
	int ret;
	enum fn_type actual_type = get_fn_type(response);
	ret = fipc_recv_msg_end(chnl, response);
	if (ret) {
		pr_err("Error finishing receipt of response, ret = %d\n", ret);
		return ret;
	} else if (actual_type != expected_type) {
		pr_err("Unexpected fn type: actual = %d, expected = %d\n",
			actual_type, expected_type);
		return -EINVAL;
	} else {
		return 0;
	}
}

static inline int finish_response_check_fn_type_and_reg0(
	struct fipc_ring_channel *chnl,
	struct fipc_message *response,
	enum fn_type expected_type,
	unsigned long expected_reg0)
{
	int ret;
	enum fn_type actual_type = get_fn_type(response);
	unsigned long actual_reg0 = fipc_get_reg0(response);

	ret = fipc_recv_msg_end(chnl, response);
	if (ret) {
		pr_err("Error finishing receipt of response, ret = %d\n", ret);
		return ret;
	} else if (actual_type != expected_type) {
		pr_err("Unexpected fn type: actual = %d, expected = %d\n",
			actual_type, expected_type);
		return -EINVAL;
	} else if (actual_reg0 != expected_reg0) {
		pr_err("Unexpected return value (reg0): actual = 0x%lx, expected = 0x%lx\n",
			actual_reg0, expected_reg0);
		return -EINVAL;

	} else {
		return 0;
	}
}

#else /* !CHECK_MESSAGES */

static inline int finish_response_check_fn_type(struct fipc_ring_channel *chnl,
						struct fipc_message *response,
						enum fn_type expected_type)
{
	return fipc_recv_msg_end(chnl, response);
}

static inline int finish_response_check_fn_type_and_reg0(
	struct fipc_ring_channel *chnl,
	struct fipc_message *response,
	enum fn_type expected_type,
	unsigned long expected_reg0)
{
	return fipc_recv_msg_end(chnl, response);
}

#endif /* CHECK_MESSAGES */

static int noinline __used
null_invocation(struct fipc_ring_channel *chan)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, NULL_INVOCATION);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type(chan, response, NULL_INVOCATION);

fail:
	return ret;
}

static int noinline __used
add_constant(struct fipc_ring_channel *chan, unsigned long trans)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, ADD_CONSTANT);
	fipc_set_reg0(request, trans);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type_and_reg0(
		chan,
		response, 
		ADD_CONSTANT,
		trans + 50);

fail:
	return ret;
}

static int noinline __used
add_nums(struct fipc_ring_channel *chan, unsigned long trans, 
	unsigned long res1)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, ADD_NUMS);
	fipc_set_reg0(request, trans);
	fipc_set_reg1(request, res1);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type_and_reg0(
		chan,
		response, 
		ADD_NUMS,
		trans + res1);

fail:
	return ret;
}

static int noinline __used
add_3_nums(struct fipc_ring_channel *chan, unsigned long trans, 
	unsigned long res1, unsigned long res2)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, ADD_3_NUMS);
	fipc_set_reg0(request, trans);
	fipc_set_reg1(request, res1);
	fipc_set_reg2(request, res2);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type_and_reg0(
		chan,
		response, 
		ADD_3_NUMS,
		trans + res1 + res2);

fail:
	return ret;
}

static int noinline __used
add_4_nums(struct fipc_ring_channel *chan, unsigned long trans, 
	unsigned long res1, unsigned long res2, unsigned long res3)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, ADD_4_NUMS);
	fipc_set_reg0(request, trans);
	fipc_set_reg1(request, res1);
	fipc_set_reg2(request, res2);
	fipc_set_reg3(request, res3);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type_and_reg0(
		chan,
		response, 
		ADD_4_NUMS,
		trans + res1 + res2 + res3);

fail:
	return ret;
}

static int noinline __used
add_5_nums(struct fipc_ring_channel *chan, unsigned long trans, 
	unsigned long res1, unsigned long res2, unsigned long res3,
	unsigned long res4)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, ADD_5_NUMS);
	fipc_set_reg0(request, trans);
	fipc_set_reg1(request, res1);
	fipc_set_reg2(request, res2);
	fipc_set_reg3(request, res3);
	fipc_set_reg4(request, res4);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type_and_reg0(
		chan,
		response, 
		ADD_5_NUMS,
		trans + res1 + res2 + res3 + res4);

fail:
	return ret;
}

static int noinline __used
add_6_nums(struct fipc_ring_channel *chan, unsigned long trans, 
	unsigned long res1, unsigned long res2, unsigned long res3,
	unsigned long res4, unsigned long res5)
{
	struct fipc_message *request;
	struct fipc_message *response;
	int ret;
	/*
	 * Set up request
	 */
	ret = test_fipc_blocking_send_start(chan, &request);
	if (ret) {
		pr_err("Error getting send message, ret = %d\n", ret);
		goto fail;
	}
	set_fn_type(request, ADD_6_NUMS);
	fipc_set_reg0(request, trans);
	fipc_set_reg1(request, res1);
	fipc_set_reg2(request, res2);
	fipc_set_reg3(request, res3);
	fipc_set_reg4(request, res4);
	fipc_set_reg5(request, res5);
	/*
	 * Send request, and get response
	 */
	ret = send_and_get_response(chan, request, &response);
	if (ret) {
		pr_err("Error getting response, ret = %d\n", ret);
		goto fail;
	}
	/*
	 * Maybe check message
	 */
	return finish_response_check_fn_type_and_reg0(
		chan,
		response, 
		ADD_6_NUMS,
		trans + res1 + res2 + res3 + res4 + res5);

fail:
	return ret;
}

int caller(void *_caller_channel_header)
{
        struct fipc_ring_channel *chan = _caller_channel_header;
	unsigned long transaction_id;
	unsigned long res1, res2, res3, res4, res5;
	unsigned long start, end;
	int ret = 0;
	/*
	 * Turn off interrupts so that we truly take over the core
	 */
	local_irq_disable();

	get_random_bytes(&res1, sizeof(res1));
	res2 = res1 + res1;
	res3 = res1 + res2;
	res4 = res3 + res2;
	res5 = res4 + res3;
	/*
	 * Do null invocations
	 */
	pr_err("Null Invocation Roundtrip Times (in cycles):\n");
	for (transaction_id = 0; 
	     transaction_id < TRANSACTIONS/2;
	     transaction_id++) {

		start = test_fipc_start_stopwatch();
		ret = null_invocation(chan);
		end = test_fipc_stop_stopwatch();

		if (ret) {
			pr_err("error doing null invocation, ret = %d, exiting...\n",
				ret);
			goto out;
		}

		pr_err("\t%lu\n", end - start);
	}
	/*
	 * Add 6 nums
	 */
	pr_err("Add 6 nums Roundtrip Times (in cycles):\n");
	for (transaction_id = TRANSACTIONS/2;
	     transaction_id < TRANSACTIONS;
	     transaction_id++) {

		start = test_fipc_start_stopwatch();
		ret = add_6_nums(chan,
				transaction_id, 
				res1, res2, res3, res4, res5);
		end = test_fipc_stop_stopwatch();

		if (ret) {
			pr_err("error doing add6nums invocation, ret = %d, exiting...\n",
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
