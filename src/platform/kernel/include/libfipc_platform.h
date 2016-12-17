/*
 * libfipc_platform.h
 *
 * Kernel-specific defs part of the public API
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_PLATFORM_H
#define LIBFIPC_PLATFORM_H

#include <linux/bug.h>
#include "libfipc_platform_internal.h"

/* BUILD CHECKS ------------------------------------------------------------ */

#define FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x) \
	(__FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x))
#define FIPC_BUILD_BUG_ON(x) \
	(__FIPC_BUILD_BUG_ON(x))

#endif /* LIBFIPC_PLATFORM_H */
