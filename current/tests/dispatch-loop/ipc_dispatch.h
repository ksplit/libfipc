#ifndef IPC_DISPATCH_H
#define IPC_DISPATCH_H

typedef struct ipc_local_fn{
    unsigned long fn_type;
    int (*local_fn)(unsigned long*, int);
} ipc_local_fn_t;

#endif
