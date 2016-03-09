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
#include <libfipc_platform.h>


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


#endif /* LIBFIPC_INTERNAL_H */
