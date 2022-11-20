all: compile_example

run: compile_example
	./a.out

WARNINGS = -Wall -Wextra -Wconversion -Werror

compile_example: example.cpp timer.h
	g++ ${WARNINGS} example.cpp -Ofast -std=c++20

compile_example_sanitized: example.cpp timer.h# debug build
	g++ ${WARNINGS} example.cpp -Ofast -std=c++20 -fsanitize=address,undefined -g
	# g++ -Wall -Wextra example.cpp -Ofast -std=c++20 -fsanitize=thread,undefined -g

test_compile: example.cpp timer.h # This is just to make sure the code compiles
	g++ ${WARNINGS} example.cpp -std=c++2a -DDISABLE_TIMER_THREADS
	g++ ${WARNINGS} example.cpp -std=c++2a -DTIMER_DEBUG
	g++ ${WARNINGS} example.cpp -std=c++2a -DDISABLE_TIMER_THREADS -DTIMER_DEBUG
	g++ ${WARNINGS} example.cpp -std=c++2a
	clang++ ${WARNINGS} example.cpp -std=c++20 -DDISABLE_TIMER_THREADS
	clang++ ${WARNINGS} example.cpp -std=c++20 -DTIMER_DEBUG
	clang++ ${WARNINGS} example.cpp -std=c++20 -DDISABLE_TIMER_THREADS -DTIMER_DEBUG
	clang++ ${WARNINGS} example.cpp -std=c++20


clean:
	rm ./a.out
