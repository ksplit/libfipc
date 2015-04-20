#include "../betaModule.h"

static unsigned long foo(unsigned long a, unsigned long b, unsigned long c)
{
	/* just some random math, doesnt mean anything just meant to take cycles */
	return (((a << 20) + a) - ((b >> 20) + b) + c)<<4;

}

unsigned long foo_dispatch(struct ipc_message *msg)
{
	return foo(msg->reg1, msg->reg6, msg->reg4);
}

unsigned long bar_dispatch(void)
{
	return 10;
}

unsigned long baz_dispatch(void)
{
	return 11;
}
