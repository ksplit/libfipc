/*
 * libfipc_internal.h
 *
 * libfipc-internal defs, not part of public API.
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_INTERNAL_H
#define LIBFIPC_INTERNAL_H

#include <libfipc_platform_types.h>
#include <libfipc_platform_internal.h>

static inline int fipc_mutex_init(fipc_mutex_t *mutex)
{
	return __fipc_mutex_init(mutex);
}

static inline int fipc_mutex_lock(fipc_mutex_t *mutex)
{
	return __fipc_mutex_lock(mutex);
}

static inline int fipc_mutex_unlock(fipc_mutex_t *mutex)
{
	return __fipc_mutex_unlock(mutex);
}

#define FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x) \
	(__FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x))
#define FIPC_BUILD_BUG_ON(x) \
	(__FIPC_BUILD_BUG_ON(x))

#endif /* LIBFIPC_INTERNAL_H */
