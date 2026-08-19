#include "../h/semaphore.hpp"
#include "../h/tcb.hpp"
#include "../h/scheduler.hpp"

void* KernelSemaphore::operator new(size_t size) {
    size_t blocks = MemoryAllocator::bytesToBlocksCeil(size);
    return MemoryAllocator::Instance()->kmemAlloc(blocks);
}
void* KernelSemaphore::operator new[](size_t size) {
    size_t blocks = MemoryAllocator::bytesToBlocksCeil(size);
    return MemoryAllocator::Instance()->kmemAlloc(blocks);
}
void KernelSemaphore::operator delete(void* p) {
    MemoryAllocator::Instance()->kmemFree(p);
}
void KernelSemaphore::operator delete[](void* p) {
    MemoryAllocator::Instance()->kmemFree(p);
}

int KernelSemaphore::wait() volatile {
    return waitN(1);
}
int KernelSemaphore::signal() volatile {
    return signalN(1);
}

int KernelSemaphore::waitN(unsigned n) volatile {
    if (n == 0) return 0;

    // FIFO: block if there is someone already in the queue OR there isn't enough available
    if (blockedThreadsQueue.peekFirst() != nullptr || val < (int)n) {
        block(n);
        if (TCB::running->isBlocked()) {
            TCB::running->setReady();
            return -1;  // sem_close case
        }
        return 0;
    }

    // queue is empty AND there is enough available - take it immediately
    val -= (int)n;
    return 0;
}

int KernelSemaphore::signalN(unsigned n) volatile {
    if (n == 0) return 0;
    val += (int)n;
    while (blockedThreadsQueue.peekFirst() &&
           val >= (int)blockedThreadsQueue.peekFirst()->getSemaphoreRequest()) {
        tryUnblock();
    }
    return 0;
}

void KernelSemaphore::block(unsigned n) volatile {
    TCB* old = TCB::running;
    old->setBlocked();
    old->setSemaphoreRequest(n);
    blockedThreadsQueue.addLast(old);
    TCB::running = Scheduler::get();
    TCB::contextSwitch(&old->context, &TCB::running->context);
}

int KernelSemaphore::tryUnblock() volatile {
    TCB* unblockedThread = blockedThreadsQueue.removeFirst();
    if (unblockedThread == 0) return -1;
    val -= (int)unblockedThread->getSemaphoreRequest();
    unblockedThread->setSemaphoreRequest(0);
    unblockedThread->setReady();
    Scheduler::put(unblockedThread);
    return 0;
}

void KernelSemaphore::unblockAll() {
    while (blockedThreadsQueue.peekFirst()) {
        TCB* unblockedThread = blockedThreadsQueue.removeFirst();
        if (!unblockedThread) break;
        unblockedThread->setSemaphoreRequest(0);
        // Not calling setReady() - wait() returns -1 (sem_close case)
        Scheduler::put(unblockedThread);
    }
}
