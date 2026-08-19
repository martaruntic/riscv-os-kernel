#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include "list.hpp"
#include "../lib/hw.h"
#include "MemoryAllocator.hpp"

class TCB;

class KernelSemaphore {
public:
    KernelSemaphore(uint32 initVal = 1) : val((int)initVal), blockedThreadsQueue() {}

    int wait() volatile;
    int signal() volatile;

    int waitN(unsigned n) volatile;
    int signalN(unsigned n) volatile;

    int value() const { return val; }

    void unblockAll();

    static void* operator new(size_t size);
    static void* operator new[](size_t size);
    static void operator delete(void* p);
    static void operator delete[](void* p);

protected:
    void block(unsigned n) volatile;
    int tryUnblock() volatile;

private:
    int val;
    List<TCB> blockedThreadsQueue;
};

#endif
