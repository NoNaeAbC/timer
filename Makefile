all: compile_example

run: compile_example
	./a.out

compile_example: example.cpp timer.h
	g++ -Wall -Wextra example.cpp -Ofast -std=c++20

compile_example_sanitized: example.cpp timer.h
	g++ -Wall -Wextra example.cpp -Ofast -std=c++20 -fsanitize=address,undefined -g

clean:
	rm ./a.out
