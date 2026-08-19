//
// Created by os on 9/6/25.
//

#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../h/tcb.hpp"
#include "../h/syscall_c.hpp"
#include "../h/MemoryAllocator.hpp"
#include "../h/semaphore.hpp"

void Riscv::popSppSpie(){

	mc_sstatus(SSTATUS_SPP);

	__asm__ volatile("csrw sepc, ra");
	__asm__ volatile("sret");

}

void Riscv::endProcess(){
	uint32 value = 0x5555;
	uint32* address = reinterpret_cast<uint32*>(0x100000UL);
	*address = value;
}

void Riscv::handleSupervisorTrap(Context* context) {

	uint64 scause = r_scause();

	if (scause == 0x09UL) {
	// ecall from S mode
	}
	else if (scause == 0x08UL) {
	// ecall from U mode
		volatile uint64 sepc = r_sepc() + 4;
		volatile uint64 sstatus = r_sstatus();

		handleSystemCalls(context);

		w_sstatus(sstatus);
		w_sepc(sepc);
	}
	else if (scause == (0x01UL << 63 | 0x01)) {
	// timer interrupt
		// The timer is used to wake up sleeping threads,
		// but we do not perform an asynchronous context switch.
		TCB::tryWakeUp();

		mc_sip(SIP_SSIP);
	}
	else if (scause == (0x01UL << 63 | 0x09)) {
	 // external hardware interrupt (console)
		console_handler();
	}
	else if (scause == 0x02UL) {
		// illegal instruction - unauthorized mode
		__putc('E');
		__putc('R');
		__putc('R');
		__putc('O');
		__putc('R');
		__putc('\n');
		w_sepc(r_sepc()+4);
	}
	else{
	// other
		endProcess();
	}

}

void Riscv::handleSystemCalls(Context* context) {
	// read the syscall id from a0(x10) and based on it call the correct kernel function
	SyscallId syscall_id = static_cast<SyscallId>(context->x[10]);
	uint64 result = 0;
	switch (syscall_id) {
		case MEM_ALLOC: {
			size_t arg1 = static_cast<size_t>(context->x[11]); // x11 = a1
			result = reinterpret_cast<uint64>(MemoryAllocator::Instance()->kmemAlloc(arg1));
			break;
		}
		case MEM_FREE: {
			void* arg1 = reinterpret_cast<void*>(context->x[11]);
			// convert int to uint64
			result = static_cast<uint64>(static_cast<long>(MemoryAllocator::Instance()->kmemFree(arg1)));
			break;
		}
		case THREAD_CREATE: {
			TCB** handle = reinterpret_cast<TCB**>(context->x[11]);
			void (*startRoutine)(void*) = reinterpret_cast<void (*)(void*)>(context->x[12]);
			void* arg = reinterpret_cast<void*>(context->x[13]);
			uint64 threadSp = context->x[14];
			if (handle == nullptr ||
				startRoutine == nullptr ||
				threadSp == 0) {
				result = static_cast<uint64>(-1);
				break;
			}
			TCB* thread = TCB::createThread(startRoutine, threadSp, arg);
			// if allocation fails signal the error!
			if (thread) {
				*handle = thread;
				(*handle)->setThreadId();
			}
			else
				result = -1; // cannot allocate thread or invalid handle

			break;
		}
		case THREAD_DISPATCH: {
			TCB::dispatch();
			break;
		}
		case THREAD_EXIT: {
			TCB::running->setFinished();
			TCB::dispatch();
			break;
		}
		case SEM_OPEN: {
			KernelSemaphore** handle = reinterpret_cast<KernelSemaphore**>(context->x[11]);
			uint32 init = static_cast<uint32>(context->x[12]);

			KernelSemaphore* sem = new KernelSemaphore(init);

			if (handle && sem)
				*handle = sem;
			else
				result = -1; // cannot allocate thread or invalid handle

			break;
		}
		case SEM_CLOSE: {
			KernelSemaphore* semHandle = reinterpret_cast<KernelSemaphore*>(context->x[11]);
			if (!semHandle) {
				result = -1;
				break;
			}
			semHandle->unblockAll();
			delete semHandle;
			break;
		}
		case SEM_WAIT: {
			KernelSemaphore* semHandle = reinterpret_cast<KernelSemaphore*>(context->x[11]);
			result = semHandle ? semHandle->wait() : (uint64)-1;
			break;
		}
		case SEM_SIGNAL: {
			KernelSemaphore* semHandle = reinterpret_cast<KernelSemaphore*>(context->x[11]);
			result = semHandle ? semHandle->signal() : (uint64)-1;
			break;
		}
		case SEM_WAIT_N: {
			KernelSemaphore* semHandle = reinterpret_cast<KernelSemaphore*>(context->x[11]);
			unsigned n = static_cast<unsigned>(context->x[12]);
			result = semHandle ? semHandle->waitN(n) : (uint64)-1;
			break;
		}
		case SEM_SIGNAL_N: {
			KernelSemaphore* semHandle = reinterpret_cast<KernelSemaphore*>(context->x[11]);
			unsigned n = static_cast<unsigned>(context->x[12]);
			result = semHandle ? semHandle->signalN(n) : (uint64)-1;
			break;
		}
		case THREAD_SLEEP: {
			uint64 time = context->x[11];
			result = TCB::asleepRunningThread(time);
			break;
		}
		case PUTC: {
			// PART FOR 20 POINTS
			char c = static_cast<char>(context->x[11]);
			__putc(c);
			break;
		}

		case GETC: {
			// PART FOR 20 POINTS
			char c = __getc();
			result = static_cast<uint64>(static_cast<uint8>(c));
			break;
		}
		case THREAD_ID: {
			result = TCB::running->getThreadId();
			break;
		}
		default: {
			result = -1;
			break; // wrong syscall id
		}
	}

	context->x[10] = result;


}
