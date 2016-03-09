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

/* DEBUGGING ---------------------------------------- */

#define FIPC_DEBUG_NONE 0
#define FIPC_DEBUG_ERR  1
#define FIPC_DEBUG_VERB 2

#define FIPC_DEBUG_LVL FIPC_DEBUG_NONE

#define fipc_debug(fmt, ...) __fipc_debug(fmt,##__VA_ARGS__)

#define FIPC_DEBUG(lvl, fmt, ...) do {				\
		if (lvl <= FIPC_DEBUG_LVL) {			\
			fipc_debug(fmt,##__VA_ARGS__);		\
		}						\
	} while (0)


/* MUTEXES -------------------------------------------------- */

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

/* BUILD CHECKS ------------------------------------------------------------ */

#define FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x) \
	(__FIPC_BUILD_BUG_ON_NOT_POWER_OF_2(x))
#define FIPC_BUILD_BUG_ON(x) \
	(__FIPC_BUILD_BUG_ON(x))

#endif /* LIBFIPC_INTERNAL_H */
