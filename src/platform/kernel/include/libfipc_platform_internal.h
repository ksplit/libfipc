/*
 * libfipc_platform_internal.h
 *
 * libfipc internal kernel-specific defs.
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_PLATFORM_INTERNAL_H
#define LIBFIPC_PLATFORM_INTERNAL_H

#include <linux/bug.h>
#include <linux/printk.h>

#define __fipc_debug(fmt, ...) \
    printk(KERN_ERR "fipc: %s:%d: "fmt,__FUNCTION__,__LINE__,##__VA_ARGS__)

#define __FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x) (BUILD_BUG_ON_NOT_POWER_OF_2(x))
#define __FIPC_BUILD_BUG_ON(x) (BUILD_BUG_ON(x))

#endif /* LIBFIPC_PLATFORM_INTERNAL_H */
