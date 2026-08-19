
#include "../h/syscall_c.hpp"
#define MAX_ARG_COUNT 7


#ifdef __cplusplus
extern "C" {
#endif


uint64 trigger_syscall(SyscallId id, uint64 args[], uint8 argc) {

	if (argc > MAX_ARG_COUNT) return -2;

	uint64 local_args[MAX_ARG_COUNT];
	uint64 ret_val;


	for (uint64 i = 0; i < argc; i++) {
		local_args[i] = args[i];
	}

	asm volatile (
		"mv a0, %1\n\t"
		"mv a1, %2\n\t"
		"mv a2, %3\n\t"
		"mv a3, %4\n\t"
		"mv a4, %5\n\t"
		"mv a5, %6\n\t"
		"mv a6, %7\n\t"
		"mv a7, %8\n\t"
		"ecall\n\t"
		"mv %0, a0"
		: "=r" (ret_val)
		: "r" (id),
		  "r" (argc >= 1 ? local_args[0] : 0),
		  "r" (argc >= 2 ? local_args[1] : 0),
		  "r" (argc >= 3 ? local_args[2] : 0),
		  "r" (argc >= 4 ? local_args[3] : 0),
		  "r" (argc >= 5 ? local_args[4] : 0),
		  "r" (argc >= 6 ? local_args[5] : 0),
		  "r" (argc >= 7 ? local_args[6] : 0)
		: "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
	);

	return ret_val;
}

void* mem_alloc(size_t size) {
	uint64 args[1];
	size_t size_in_blocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
	args[0] = (uint64)size_in_blocks;
	return (void*)trigger_syscall(MEM_ALLOC, args, 1);
}

int mem_free(void* addr) {
	uint64 args[1];
	args[0] = (uint64)addr;
	return (int)(long)trigger_syscall(MEM_FREE, args, 1);
}


int thread_create(thread_t* handle, void (*start_routine)(void*), void* arg) {
	// Without a valid handle and function, we don't attempt to create the thread.
	if (handle == nullptr || start_routine == nullptr) {
		return -1;
	}
	uint64 args[4];

	void* stack = mem_alloc(DEFAULT_STACK_SIZE) ;

	if (!stack) return -1; // unable to allocate stack

	void* thread_stack_pointer = (char*)stack + DEFAULT_STACK_SIZE; // point to the last (biggest) address in allocated stack

	args[0] = (uint64)handle;
	args[1] = (uint64)start_routine;
	args[2] = (uint64)arg;
	args[3] = (uint64)thread_stack_pointer;

	int status = (int)(long)trigger_syscall(THREAD_CREATE, args, 4);
	if (status < 0) mem_free(stack);
	return status;
}

void thread_dispatch() {
	trigger_syscall(THREAD_DISPATCH, 0, 0);
}

int thread_exit() {
	return (int)(long)trigger_syscall(THREAD_EXIT, 0, 0);
}

int time_sleep(time_t time) {
	uint64 args[1];
	args[0] = (uint64)time;
	return (int)(long)trigger_syscall(THREAD_SLEEP, args, 1);
}

int sem_open(sem_t* handle, unsigned init) {
	uint64 args[2];
	args[0] = (uint64)handle;
	args[1] = (uint64)init;

	return (int)(long)trigger_syscall(SEM_OPEN, args, 2);
}

int sem_close(sem_t handle) {
	uint64 args[1];
	args[0] = (uint64)handle;

	return (int)(long)trigger_syscall(SEM_CLOSE, args, 1);
}

int sem_wait(sem_t id) {
	uint64 args[1];
	args[0] = (uint64)id;

	return (int)(long)trigger_syscall(SEM_WAIT, args, 1);
}

int sem_signal(sem_t id) {
	uint64 args[1];
	args[0] = (uint64)id;

	return (int)(long)trigger_syscall(SEM_SIGNAL, args, 1);
}

int sem_wait_n(sem_t id, unsigned n) {
	uint64 args[2];
	args[0] = (uint64)id;
	args[1] = (uint64)n;

	return (int)(long)trigger_syscall(SEM_WAIT_N, args, 2);
}

int sem_signal_n(sem_t id, unsigned n) {
	uint64 args[2];
	args[0] = (uint64)id;
	args[1] = (uint64)n;

	return (int)(long)trigger_syscall(SEM_SIGNAL_N, args, 2);
}

void putc(char c) {
	uint64 args[1];
	args[0] = (uint64)(uint8)c;
	trigger_syscall(PUTC, args, 1);
}

char getc() {
	return (char)(uint8)trigger_syscall(GETC, 0, 0);
}

int getThreadId(){
	uint64 args[1];
	return (int)(long)trigger_syscall(THREAD_ID,args,0);
}

#ifdef __cplusplus
}
#endif