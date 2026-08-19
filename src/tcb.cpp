//
// Created by os on 9/6/25.
//

#include "../h/tcb.hpp"
#include "../h/syscall_c.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../lib/console.h"

TCB* TCB::running = nullptr;
TCB* TCB::asleepList = nullptr;
TCB* TCB::ghost = nullptr;
TCB* TCB::zombieList = nullptr;
unsigned TCB::maxThreads = 0;
KernelSemaphore* TCB::MaxThreadSem = nullptr;

extern KernelSemaphore* sem;
void userMainWrapper(void*);

void* TCB::operator new(size_t size) {
    size_t blocks = MemoryAllocator::bytesToBlocksCeil(size);
    return MemoryAllocator::Instance()->kmemAlloc(blocks);
}

void* TCB::operator new[](size_t size) {
    size_t blocks = MemoryAllocator::bytesToBlocksCeil(size);
    return MemoryAllocator::Instance()->kmemAlloc(blocks);
}

void TCB::operator delete(void* p) {
    MemoryAllocator::Instance()->kmemFree(p);
}

void TCB::operator delete[](void* p) {
    MemoryAllocator::Instance()->kmemFree(p);
}

TCB* TCB::createThread(Body body, uint64 sp, void* arg, PrivilegeLevel mode) {
    return new TCB(body, sp, arg, mode);
}

TCB::TCB(Body body, uint64 sp, void* arg, PrivilegeLevel mode)
    : body(body),
      arg(arg),
      stackBound(body ? sp - DEFAULT_STACK_SIZE : 0),
      context({
          (uint64)&threadWrapper,
          sp
      }),
      status(ThreadStatus::READY),
      runMode(mode),
      relTimeAsleep(0),
      nextAsleep(nullptr),
      nextZombie(nullptr),
      semaphoreRequest(0)
{
    if (body && body != TCB::idleThread) {
        // if the TCB has a function to run, put it in the ready queue
        Scheduler::put(this);
        // idle must not be added to the ready queue automatically; the Scheduler only uses it when ready is empty
    }
}

void TCB::dispatch() {
    TCB* old = running;

    if (old->isFinished()) {
        if (zombieList) {

            old->nextZombie = zombieList;
        }
        zombieList = old;
    }
    else if (!old->isBlocked() && old->body) {
        Scheduler::put(old);
    }

    running = Scheduler::get();

    TCB::contextSwitch(&old->context, &running->context);
}

int TCB::asleepRunningThread(time_t timeAsleep) {
    if (timeAsleep == 0) return -1;

    running->relTimeAsleep = timeAsleep;
    running->nextAsleep = asleepList;
    running->setBlocked();

    asleepList = running;

    dispatch();

    return 0;
}

void TCB::tryWakeUp() {
    TCB* prev = nullptr;
    TCB* curr = asleepList;

    while (curr) {
        if (curr->relTimeAsleep > 0) {
            curr->relTimeAsleep--;
        }

        if (curr->relTimeAsleep == 0) {
            TCB* wake = curr;

            if (prev) {
                prev->nextAsleep = curr->nextAsleep;
            } else {
                asleepList = curr->nextAsleep;
            }

            curr = curr->nextAsleep;

            wake->nextAsleep = nullptr;
            wake->setReady();

            Scheduler::put(wake);
        } else {
            prev = curr;
            curr = curr->nextAsleep;
        }
    }
}

void TCB::yield() {
    Riscv::pushRegisters();

    uint64 sepc = Riscv::r_sepc();
    uint64 sstatus = Riscv::r_sstatus();

    dispatch();

    Riscv::w_sstatus(sstatus);
    Riscv::w_sepc(sepc);

    Riscv::popRegisters();
}

void TCB::threadWrapper() {

    void* par = running->arg;

    bool isUserThread = running->getRunMode() == TCB::PrivilegeLevel::USER;
    if(isUserThread){
        if(MaxThreadSem) MaxThreadSem->wait();
        Riscv::popSppSpie();
    }

    (running->body)(par);
    if(isUserThread && MaxThreadSem) MaxThreadSem->signal();

    running->setFinished();

    if (running->body == userMainWrapper) {
        sem->signal();
    }

    thread_dispatch();
}

void TCB::idleThread(void*) {
    while (true) {
        TCB::dispatch();
    }
}