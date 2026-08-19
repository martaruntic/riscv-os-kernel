//
// Created by os on 9/10/25.
//

#include "../h/Console.hpp"
#include "../lib/hw.h"
#include "../h/tcb.hpp"
#include "../h/syscall_c.hpp"

Console* Console::Instance() {
	static Console c;
	return &c;
}

// System call implementation that puts char c in wBuff
void Console::kputc(char c) {
	// the producer is user thread requesting to write to console
	wBuff.append(c);
}

char Console::kgetc() {
	char c = rBuff.pop();
	return c;
}

// This will be run as the consumer kernel thread in S mode for wbuff
void Console::writeToController(void* cons) {
	volatile Console* console = (Console*) cons;
	while(true){

		volatile char c = (console->wBuff).pop();


		// polling until console controller is ready to receive next char
		while(!(*(uint8*)(CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT));

		*(char*)(CONSOLE_TX_DATA) = c;

	}
}

// potential run this thread when there are no others in scheduler
void Console::ghostThread(void* x) {
	// this thread is run in a U mode and can be interrupted bu the console controler
	// after console handler has finished it has to unblock the user thread waiting on input

	while(true) {
	}
}

void Console::consoleHandler() {

	while(*(uint8*)(CONSOLE_STATUS) & CONSOLE_RX_STATUS_BIT) {
		char c = *(char*)(CONSOLE_RX_DATA);
		if(c == '\r') c = '\n';
		// no synchronization here, since I must not block the interrupted thread
		// I'll just drop characters or whatever instead
		rBuff.nonBlockingAppend(c);
	}
}




