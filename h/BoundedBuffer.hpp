//
// Created by os on 9/10/25.
//

#ifndef BOUNDEDBUFFER_HPP
#define BOUNDEDBUFFER_HPP

#include "semaphore.hpp"

const int N = 256 ; // max buffer size

class BoundedBuffer {
public:
	BoundedBuffer();

	void append(char c);

	char pop() volatile;

	void nonBlockingAppend(char c);

private:
	KernelSemaphore spaceAvailable, itemAvailable;

	char buffer[N];
	int head, tail, size;

};
#endif //PROJECT_BASE_BOUNDEDBUFFER_HPP