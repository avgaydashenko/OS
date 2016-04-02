#ifndef __THREAD_H__
#define __THREAD_H__

#include <sys/types.h>

#define MAX_THREAD (1 << 10)

void thread_pool_init();

pid_t create_thread(void* (*fptr)(void *), void *arg);
pid_t get_thread();

void yield();
void join(pid_t thread, void** retvalue);

#endif /* __THREAD_H__ */