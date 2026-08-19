#ifndef PROJECT_BASE_SYSCALL_C_H
#define PROJECT_BASE_SYSCALL_C_H

#include "../lib/hw.h"

#ifdef __cplusplus
extern "C" {
#endif

class _thread;
typedef _thread* thread_t;

typedef unsigned long time_t;

class _sem;
typedef _sem* sem_t;

const int EOF = -1;

typedef enum {
    MEM_ALLOC = 0x01,
    MEM_FREE = 0x02,
    THREAD_CREATE = 0x11,
    THREAD_EXIT = 0x12,
    THREAD_DISPATCH = 0x13,
    SEM_OPEN = 0x21,
    SEM_CLOSE = 0x22,
    SEM_WAIT = 0x23,
    SEM_SIGNAL = 0x24,
    SEM_WAIT_N = 0x25,
    SEM_SIGNAL_N = 0x26,
    THREAD_SLEEP = 0x31,
    GETC = 0x41,
    PUTC = 0x42,
	THREAD_JOIN = 0x32,
} SyscallId;

uint64 trigger_syscall(SyscallId id, uint64* args, uint8 argc);

void* mem_alloc(size_t size);
int mem_free(void*);
int thread_create(thread_t* handle, void (*start_routine)(void*), void* arg);
int thread_exit();
void thread_dispatch();
int time_sleep(time_t);

int sem_open(sem_t* handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);
int sem_wait_n(sem_t id, unsigned n);
int sem_signal_n(sem_t id, unsigned n);

char getc();
void putc(char);
int thread_join(thread_t handle);

#ifdef __cplusplus
}
#endif

#endif // PROJECT_BASE_SYSCALL_C_H
