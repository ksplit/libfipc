/*
 * libfipc_platform_internal.h
 *
 * libfipc internal kernel-specific defs.
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_PLATFORM_INTERNAL_H
#define LIBFIPC_PLATFORM_INTERNAL_H

#include <linux/mutex.h>
#include <linux/bug.h>
#include <linux/printk.h>
#include <libfipc_platform_types.h>

#define __fipc_debug(fmt, ...) \
    printk(KERN_ERR "fipc: %s:%d: "format,__FUNCTION__,__LINE__,##__VA_ARGS__)

static inline int __fipc_mutex_init(fipc_mutex_t *mutex)
{
	mutex_init(mutex);
	return 0;
}

static inline int __fipc_mutex_lock(fipc_mutex_t *mutex)
{
	mutex_lock(mutex);
	return 0;
}

static inline int __fipc_mutex_unlock(fipc_mutex_t *mutex)
{
	mutex_unlock(mutex);
	return 0;
}

#define __FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x) (BUILD_BUG_ON_NOT_POWER_OF_2(x))
#define __FIPC_BUILD_BUG_ON(x) (BUILD_BUG_ON(x))

#endif /* LIBFIPC_PLATFORM_INTERNAL_H */
