# Threads v1.0
 A cooperative multithreading library for AVR
 
# Features
- Create up to 65k threads (if your RAM can handle it)
- Easily transfer control to other threads with a single function call
- Destroy threads after use

# Easy to use
To get ready for using threads, simply call `Threads::init(defaultStackSize);`  
To create a new thread, call `Threads::createThread(newThread, stackSize = -1);`  
To switch to the next thread, call `Threads::yield();`  
To destroy another thread, call `Threads::destroyThread(pid);`  
To destroy current thread, return the function.

A simple program might look like this:
```
THREAD workerThread(void) {
   while (1) {
        work();
        Threads::yield();
    }
}

THREAD blinkerThread(void) {
    while (1) {
        blinkLED();
        Threads::yield();
    }
}

int main(void) {
    Threads::init(128);
    Threads::createThread(workerThread);
    Threads::createThread(blinkerThread);
    
    while (1) {
        prepareWork();
        Threads::yield();
        outputWork();
    }
}
```
A short manual can be found in ["Manual.md"](https://github.com/Bliskit/avr-threads/blob/master/Manual.md).

# Arduino-compatible
Threads does not use any timers or other peripherals, so using it with the Arduino IDE is as simple as including the header. 
An example is provided in `multithreading.ino`  
Some tipps and tricks for using Threads with the Arduino library can be found in ["Arduino.md"](https://github.com/Bliskit/avr-threads/blob/master/Arduino.md).
