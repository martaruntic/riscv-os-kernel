//
// Created by os on 9/6/25.
//

#include "../h/scheduler.hpp"
#include "../h/tcb.hpp"

List<TCB> Scheduler::readyThreadQueue;
int Scheduler::size = 0;

TCB* Scheduler::get() {
	if(size == 0) return TCB::ghost;
	size--;
	return readyThreadQueue.removeFirst();

}

void Scheduler::put(TCB* tcb) {
	if(tcb == TCB::ghost) return;
	size++;
	readyThreadQueue.addLast(tcb);
}