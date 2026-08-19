//
// Created by os on 9/6/25.
//

#ifndef TCB_HPP
#define TCB_HPP

#include "../lib/hw.h"
#include "scheduler.hpp"
#include "riscv.hpp"
#include "semaphore.hpp"
#include "MemoryAllocator.hpp"

class TCB {
public:
	friend class Riscv;
	friend class KernelSemaphore;
	using Body = void (*)(void*); // thread function
	using time_t = unsigned long;

	enum class ThreadStatus {
		READY,
		FINISHED,
		BLOCKED
	};

	enum class PrivilegeLevel {
		USER,
		SUPERVISOR
	};
	// constructor is protected, so threads are created through this function
	static TCB* createThread(Body body, uint64 sp, void* arg = nullptr, PrivilegeLevel mode = PrivilegeLevel::USER);
	~TCB() = default;

	static TCB* running;

	static TCB* ghost;
	// list of threads that called time_sleep()
	static TCB* asleepList;
	// list of threads that have finished, but whose memory has not been freed yet
	static TCB* zombieList;

	static void yield();

	bool isFinished() const { return status == ThreadStatus::FINISHED; }

	bool isBlocked() const { return status == ThreadStatus::BLOCKED; }

	bool isReady() const { return status == ThreadStatus::READY; }

	void setFinished() { status = ThreadStatus::FINISHED; }

	void setBlocked() { status = ThreadStatus::BLOCKED; }

	void setReady() { status = ThreadStatus::READY; }

	unsigned getSemaphoreRequest() const { return semaphoreRequest; }
	void setSemaphoreRequest(unsigned n) { semaphoreRequest = n; }

	TCB* getNextZombie() { return nextZombie; }

	void* getStackStart() { return reinterpret_cast<void*>(stackBound); }

	PrivilegeLevel getRunMode() volatile { return runMode; }

	static void* operator new(size_t size);

	static void* operator new[](size_t size);

	static void operator delete(void* p);

	static void operator delete[](void* p);

	static void idleThread(void*);

	void addChild(TCB* child){
		child->nextSibling = this->firstChild;
		this->firstChild = child;
	}
	void joinAllChildren();

protected:
	struct Context {
		uint64 ra; // return address
		uint64 sp;
	}; // remembers where the thread left off

	TCB (Body body, uint64 sp, void* arg = nullptr, PrivilegeLevel mode = PrivilegeLevel::USER);

	static void threadWrapper();

	static void contextSwitch(Context* oldContext, Context* newContext);

	static void dispatch();

	static int asleepRunningThread(time_t timeAsleep);

	static void tryWakeUp();

private:

	Body body;

	void* arg;
	// start of the stack
	uint64 stackBound;

	Context context;
	// ready, blocked, finished
	ThreadStatus status;

	PrivilegeLevel runMode; // 0 for user mode 1 for supervisor mode

	time_t relTimeAsleep; // how long the thread should sleep

	TCB* nextAsleep; // pointer that forms a singly linked list of sleeping threads

	TCB* nextZombie;

	unsigned semaphoreRequest;
	// joinSem/joinWaitingCount su deo modifikacije "join obican" (feature/thread-join),
	// ali su ovde potrebni jer ih joinAllChildren() koristi da ceka svako dete
	KernelSemaphore* joinSem;
	unsigned joinWaitingCount;
	TCB* firstChild;
	TCB* nextSibling;
	TCB* parent;
	unsigned childCnt;

};



#endif