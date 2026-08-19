//
// Created by os on 9/10/25.
//

#include "../h/BoundedBuffer.hpp"


BoundedBuffer::BoundedBuffer() : spaceAvailable(N), itemAvailable(0), head(0), tail(0), size(0) {}

void BoundedBuffer::append(char c) {
	// producer thread will wait here if the buffer is full
	spaceAvailable.wait();

	buffer[tail] = c;
	tail = (tail + 1) % N;
	size++;

	// unblock consumer thread if buffer was empty and there were threads waiting
	itemAvailable.signal();
}

void BoundedBuffer::nonBlockingAppend(char c) {
	if (size < N) {
		buffer[tail] = c;
		tail = (tail + 1) % N;
		size++;

		itemAvailable.signal();
	}
}

char BoundedBuffer::pop() volatile {
	// consumer thread will wait here if the buffer is full
	itemAvailable.wait();

	char c = buffer[head];
	head = (head + 1) % N;
	size--;
	// unblock producer thread if it was waiting for free place in buffer
	spaceAvailable.signal();

	return c;
}