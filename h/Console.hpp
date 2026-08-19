//
// Created by os on 9/10/25.
//

#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "BoundedBuffer.hpp"

class  Console {

public:
	static Console* Instance();

	void kputc(char c);

	char kgetc();

	static void writeToController(void*);

	// this thread will run in backgrond in U mode
	// allowing the console controler to interrupt it
	static void ghostThread(void*);

	void consoleHandler();

	BoundedBuffer& getReadBuffer() { return rBuff; }
protected:
	Console(){}

private:
	BoundedBuffer wBuff;
	BoundedBuffer rBuff;


};

#endif //PROJECT_BASE_CONSOLE_HPP