#include "../h/syscall_cpp.hpp"

Thread::Thread(void (*body)(void*), void* arg) : myHandle(nullptr), body(body), arg(arg) {}

Thread::Thread() : myHandle(nullptr), body(nullptr), arg(nullptr) {}

Thread::~Thread() = default;

int Thread::start() {
    if (!body) {
        return thread_create(&myHandle, &Thread::wrapper, this);
    }
    return thread_create(&myHandle, body, arg);
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t time) {
    return time_sleep(time);
}

void Thread::wrapper(void* threadObj) {
    Thread* thread = static_cast<Thread*>(threadObj);
    thread->run();
}

Semaphore::Semaphore(unsigned init) : myHandle(nullptr) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

PeriodicThread::PeriodicThread(time_t period) : Thread(), period(period) {}

void PeriodicThread::run() {
    while (period > 0) {
        periodicActivation();
        if (period > 0) time_sleep(period);
    }
}

void PeriodicThread::terminate() {
    period = 0;
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}
