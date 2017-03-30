#ifndef THREAD_FN_UTIL_H
#define THREAD_FN_UTIL_H

#define ADD_2_FN 1
#define ADD_10_FN 2
#define TRANSACTIONS 3000
#define THD3_INTERVAL 100

int thread1_fn1(void* data);
int thread2_fn1(void* data);
int thread3_fn1(void* data);

#endif
