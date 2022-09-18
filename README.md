# Timer

Easily debug execution times in your code. Mark code sections with a start and end time, and print the time elapsed
between them.
Not designed to measure performance like Google benchmark.
Out if convenience it's header only. It currently requires C++17, but this may change to C++20 or C++23.
It is only guaranteed to work with the most recent version of GCC and Clang.

## usage

See example.cpp for usage.

### CodeSectionTimer

The purpose of the code section timer is, to log the execution time of the function.
To use it put CODE_SECTION_TIMER; at the beginning of the function.

### Timer

The timer logs the execution time from the start of "Timer.initialize();" on every call to "Timer.add("event name");".
Print the log the execution time with "Timer.log();".

## Code formatting

The code is formatted with clang-format. The configuration is in .clang-format. Structs use CamelCase, functions and
variables use snake_case. clang-format is required to contribute to this project.

## Comment style

Comments starting with /** are useful for the user of the library and usually mark functions you should.
/* comments are intended for contributors and developers and usually mark functions you shouldn't.
All comments must follow the following style:

```cpp
/**
 * This is a comment for the user of the library.
 */
```
They also need to be indented with tabs onto the current indentation level.
