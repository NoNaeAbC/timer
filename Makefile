all: compile_example

run: compile_example
	./a.out

compile_example: example.cpp timer.h
	g++ -Wall -Wextra example.cpp -Ofast -std=c++20

compile_example_sanitized: example.cpp timer.h
	g++ -Wall -Wextra example.cpp -Ofast -std=c++20 -fsanitize=address,undefined -g

test_compile: example.cpp timer.h # This is just to make sure the code compiles
	g++ -Wall -Wextra -Werror example.cpp -std=c++2a -DDISABLE_TIMER_THREADS
	g++ -Wall -Wextra -Werror example.cpp -std=c++2a -DTIMER_DEBUG
	g++ -Wall -Wextra -Werror example.cpp -std=c++2a -DDISABLE_TIMER_THREADS -DTIMER_DEBUG
	g++ -Wall -Wextra -Werror example.cpp -std=c++2a
	clang++ -Wall -Wextra -Werror example.cpp -std=c++20 -DDISABLE_TIMER_THREADS
	clang++ -Wall -Wextra -Werror example.cpp -std=c++20 -DTIMER_DEBUG
	clang++ -Wall -Wextra -Werror example.cpp -std=c++20 -DDISABLE_TIMER_THREADS -DTIMER_DEBUG
	clang++ -Wall -Wextra -Werror example.cpp -std=c++20


clean:
	rm ./a.out
