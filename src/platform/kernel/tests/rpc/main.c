/*
 * main.c
 *
 * Contains init/exit for rpc test
 */

#include <libfipc.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include "../test_helpers.h"
#include "rpc.h"

#define CALLER_CPU 1
#define CALLEE_CPU 3
#define CHANNEL_ORDER 2 /* channel is 2^CHANNEL_ORDER pages */

MODULE_LICENSE("GPL");

static int setup_and_run_test(void)
{
	int ret;
	struct fipc_ring_channel *caller_header, *callee_header;
	struct task_struct *caller_thread, *callee_thread;
	/*
	 * Set up channel
	 */
	ret = create_channel(CHANNEL_ORDER, &caller_header, &callee_header);
	if (ret) {
		pr_err("Error creating channel, ret = %d", ret);
		goto fail1;
	}
	/*
	 * Set up threads
	 */
	caller_thread = spawn_thread_with_channel(caller_header, caller,
						CALLER_CPU);
	if (!caller_thread) {
		pr_err("Error setting up caller thread");
		goto fail2;
	}
	callee_thread = spawn_thread_with_channel(callee_header, callee,
						CALLEE_CPU);
	if (!callee_thread) {
		pr_err("Error setting up callee thread");
		goto fail3;
	}
	/*
	 * Wake them up; they will run until they exit.
	 */
	wake_up_process(caller_thread);
	wake_up_process(callee_thread);
	/*
	 * Release our reference on them since we are going to exit
	 * before they finish
	 */
	release_thread(caller_thread);
	release_thread(callee_thread);

	return 0;

fail3:
	release_thread(caller_thread);
fail2:
	free_channel(CHANNEL_ORDER, caller_header, callee_header);
fail1:
	return ret;
}

static int __init rpc_init(void)
{
	int ret = 0;

	ret = setup_and_run_test();

        return ret;
}
static int __exit rpc_rmmod(void)
{

	return 0;
}

module_init(rpc_init);
module_exit(rpc_rmmod);
