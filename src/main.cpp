#include "../lib/console.h"
#include "../lib/hw.h"
#include "../h/MemoryAllocator.hpp"
#include "../h/syscall_c.hpp"
#include "../h/tcb.hpp"
#include "../h/riscv.hpp"



void userMain();


void userMainWrapper(void*) {
	userMain();
}
void deleteThread(TCB* t) {
	if (!t) return;
	// We must save the stack address before freeing the TCB object.
	void* stackStart = t->getStackStart();
	if (stackStart) {
		MemoryAllocator::Instance()->kmemFree(stackStart);
	}
	MemoryAllocator::Instance()->kmemFree(t); // free the stack, then the TCB
}

TCB* allocateAndCreateThread(
	void (*body)(void*),
	void* arg = nullptr,
	TCB::PrivilegeLevel mode = TCB::PrivilegeLevel::USER
) {
	void* stack = nullptr;
	uint64 stackPointer = 0;

	if (body != nullptr) {
		size_t blocks =
			MemoryAllocator::bytesToBlocksCeil(DEFAULT_STACK_SIZE);

		stack = MemoryAllocator::Instance()->kmemAlloc(blocks);

		if (!stack) {
			return nullptr;
		}

		stackPointer =
			reinterpret_cast<uint64>(
				static_cast<char*>(stack) + DEFAULT_STACK_SIZE
			);
	}

	TCB* thread = TCB::createThread(
		body,
		stackPointer,
		arg,
		mode
	);

	if (!thread && stack) {
		MemoryAllocator::Instance()->kmemFree(stack);
	}

	return thread;
}

KernelSemaphore* sem;

void main() {
	// go here whenever an interrupt or system call happens
	Riscv::w_stvec((uint64)&Riscv::supervisorTrap); // write to the register

	// used so the main thread waits for the user program to finish
	sem = new KernelSemaphore(0);

	// TCB for the main thread (the kernel thread currently executing)
	TCB* mainThread = allocateAndCreateThread(nullptr);

	// creates the thread that runs userMain
	TCB* userThread = allocateAndCreateThread(&userMainWrapper);

	// ghost thread that runs when there are no ready threads
	TCB* ghostThread = allocateAndCreateThread(
		&TCB::idleThread,
		nullptr,
		TCB::PrivilegeLevel::SUPERVISOR
	);

	TCB::ghost = ghostThread;

	if (!sem || !mainThread || !userThread || !ghostThread) {
		if (userThread) deleteThread(userThread);

		if (ghostThread) {
			deleteThread(ghostThread);
			TCB::ghost = nullptr;
		}

		if (mainThread) {
			MemoryAllocator::Instance()->kmemFree(mainThread);
		}

		if (sem) delete sem;

		Riscv::endProcess();
		return;
	}

	TCB::running = mainThread; // set it as the currently running thread

	sem->wait(); // main now waits, CPU switches to userMain

	while (TCB::zombieList) { // clean up memory - all threads that have finished
		TCB* previous = TCB::zombieList;
		TCB::zombieList = TCB::zombieList->getNextZombie();
		deleteThread(previous);
	}

	deleteThread(ghostThread);
	TCB::ghost = nullptr;

	delete sem;
	sem = nullptr;
	MemoryAllocator::Instance()->kmemFree(mainThread);

	Riscv::endProcess(); // tell the emulator that the kernel has finished
}