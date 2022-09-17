#include "timer.h"

#include <chrono>
#include <iostream>
#include <thread>

void function() { CODE_SECTION_TIMER; }

int main() {
	std::cout << "Timer example:\n";
	Timer<const char *> timer{};
	timer.initialize();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	timer.add("First measurement").print_current();
	timer.add("Second measurement").print_current();
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	timer.add("third measurement");
	timer.add("Last measurement");

	timer.log();

	std::cout << "\nCode section example:\n";
	function();
}

/*
    Example output:
    
Timer example:
Timer : First measurement after 1.00024s at 1.00024s
Timer : Second measurement after 0.147825ms at 1.00039s
Timer :
        First measurement after 1.00024s at 1.00024s
        Second measurement after 0.147825ms at 1.00039s
        third measurement after 0.200225s at 1.20061s
        Last measurement after 5.23µs at 1.20062s

Code section example:
Code section : void function() took 87ns 🠔 Note that the timing itself takes a lot of time
*/
