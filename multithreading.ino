#include "threads.hpp"

void tdelay(uint32_t timeout)
{
    uint32_t mark = millis();
    uint32_t interval;
    do {
        Threads::yield();
        interval = millis() - mark;    // prevent overflow faults
    } while(interval < timeout);
}

THREAD t1(void) {
	while (1) {
		Serial.println("T1 Loop");
		tdelay(2500);
	}
}

THREAD t2(void) {
	while (1) {
		digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    		tdelay(250);                      // wait
    		digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    		tdelay(250);                      // wait
	}
}

THREAD worker(void) {
	int i = 0;
	while (1) {
		Serial.println("Working!");
		i++;
		if (i >= 5) {
			Serial.println("Worker done!");
			return;
		}
		tdelay(500);
	}
}

void setup() {
  	pinMode(LED_BUILTIN, OUTPUT);
	
	Serial.begin(9600);
	Threads::init(128);
	Threads::createThread(t1);
	Threads::createThread(t2);
	Threads::createThread(worker);
	
	Threads::Thread *thread = Threads::currentThread;
	Serial.println("[" + String(thread->pid) + "]:0x" + String((uint16_t) thread,HEX));
	while (thread->next != Threads::currentThread) {
		thread = thread->next;
		Serial.println("[" + String(thread->pid) + "]:0x" + String((uint16_t) thread,HEX));
	}
}

int count = 0;
void loop() {
	if (Serial.available()) {
		String echo = Serial.readStringUntil('\n'); //read string from arduino serial monitor, make sure line ending is set to 'New Line'
		Serial.println("Echo: " + echo);
	}
	Threads::yield();
}
