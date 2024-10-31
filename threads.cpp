#include <avr/io.h>
#include "threads.hpp"
#include "stack_magic.hpp"

namespace Threads {
	struct Settings settings;
	
	Thread *currentThread;
	
	// Primary management functions
	void init(uint16_t defaultStackSize) {
		settings.defaultStackSize = defaultStackSize;
		currentThread = new Thread;
		currentThread->pid = 0;
		currentThread->stackptr = SP;
		currentThread->stackbase = (uint8_t*) RAMEND;
		currentThread->next = currentThread;
	}
	
	PID createThread(void (*func)(void), uint16_t stackSize = -1) {
		// Allocate required resources
    if (stackSize == -1)
      stackSize = settings.defaultStackSize;
		uint8_t *newStack = new uint8_t[stackSize];
		Thread *newThread = new Thread;
		
		// Prepare thread
		newThread->pid = getNextPID();
		newThread->stackbase = newStack;
		
		// Get adjusted stack pointer and write entry address
		uint8_t* stackptr = initStack(newStack, stackSize,func);
		newThread->stackptr = (uint16_t) stackptr;
		
		// Insert the new thread into the queue
		getLastThread()->next = newThread;
		newThread->next = currentThread;
		
		// Return PID for management purposes
		return newThread->pid;
	}
	
	void destroyThread(PID pid) {
		// Get selected thread
		Thread *selected = getThreadByPID(pid);
		if (selected == nullptr) {
			return;
		}
		
		// Get previous thread and close gap in queue
		Thread *prev = currentThread;
		while (prev->next != selected) {
			prev = prev->next;
		}
				
		prev->next = selected->next;
		
		// Free allocated memory for stack and thread
		delete selected->stackbase;
		delete selected;		
	}
	
	void yield() {    
		//delete all next threads if they are signaled done
    		while(currentThread->next->workerDone == 1) 
      			destroyThread(currentThread->next->pid);
    
		SM_SAVE_CONTEXT()

		// Save stack of current thread
		currentThread->stackptr = SP;
		
		// Switch threads
		currentThread = currentThread->next;
		
		// Restore stack of currentThread
		// As this is a critical 16 bit value, we cannot let interrupts occur
		asm("cli");
		SP = currentThread->stackptr;
		
		SM_RESTORE_CONTEXT()
	}
	
	// Secondary management functions
	PID getNextPID() {
		uint16_t highestPID = 0;
		Thread *thread = currentThread;
		while (thread->next != currentThread) {
			thread = thread->next;
			if (thread->pid > highestPID) {
				highestPID = thread->pid;
			}
		}
		return highestPID + 1;
	}
	
	Thread *getLastThread() {
		Thread *thread = currentThread;
		while (thread->next != currentThread) {
			thread = thread->next;
		}
		return thread;
	}		
	
	uint8_t *initStack(uint8_t* stackbase, uint16_t stackSize, void (*entry)(void)) {
		// We need to jump to the top of the stack and need a pointer
		uint8_t *ptr = (uint8_t*) (stackbase + stackSize - 1);
		
		
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
		//The ATmega2560/2561 have more than 128kb of flash storage, so the program counter needs three bytes instead of two for smaller AVR devices
		// Write exit return address
		__uint24 exitAddress = (__uint24) Threads::exit;
		ptr[0] = (uint8_t) exitAddress;
		ptr[-1] = (uint8_t) (exitAddress >> 8);
		ptr[-2] = (uint8_t) (exitAddress >> 16);
		ptr -= 3;
		
		// Write entry address
		__uint24 entryAddress = (__uint24) entry;
		ptr[0] = (uint8_t) entryAddress;
		ptr[-1] = (uint8_t) (entryAddress >> 8);
		ptr[-2] = (uint8_t) (entryAddress >> 16);
		ptr -= 3;
#else
		// Write exit return address
		uint16_t exitAddress = (uint16_t) Threads::exit;
		ptr[0] = (uint8_t) exitAddress;
		ptr[-1] = (uint8_t) (exitAddress >> 8);
		ptr -= 2;
		
		// Write entry address
		uint16_t entryAddress = (uint16_t) entry;
		ptr[0] = (uint8_t) entryAddress;
		ptr[-1] = (uint8_t) (entryAddress >> 8);
		ptr -= 2;
#endif
		
		// Simulate 32 pushed registers
		for (int i = 0; i < 32; i++) {
			ptr[-i] = 0;
		}
		ptr -= 32;
		
		// Write pushed SREG
		ptr[0] = SREG;
		ptr--;
		
		// This should be it
		return ptr;
	}
		
	void exit(void) {
		while (1) {
			currentThread->workerDone = 1;
			yield();
		}
	}	

	Thread *getThreadByPID(PID pid) {
		Thread *thread = currentThread;
		while (thread->next != currentThread) {
			thread = thread->next;
			if (thread->pid == pid) {
				return thread;
			}
		}
		return nullptr;
	}
}
