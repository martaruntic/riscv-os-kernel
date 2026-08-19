//
// Created by os on 9/6/25.
//

#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "list.hpp"

class TCB;

class Scheduler {

private:
	static List<TCB> readyThreadQueue;
public:
	static int size;
	static TCB* get();
	static void put(TCB* tcb);

};

#endif