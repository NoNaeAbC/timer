#ifndef TIMER_H
#define TIMER_H

#if __cplusplus < 201703L
#error "Use C++17 implementation"
#endif

#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


/*

Quick and dirty timing tool.

*/

/*
TODO:
    - loop sections
    - printing formats
    - multithreaded flow graph
*/


inline uint64_t get_time_ns() {
	using namespace std::chrono;
	return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

template<class NAME_TYPE = int>
struct TimeStamp {
	const NAME_TYPE name;
	const uint64_t  time_stamp = get_time_ns();

	explicit TimeStamp(NAME_TYPE id) : name(id) {}

	static uint64_t get_diff(TimeStamp first, TimeStamp last) { return last.time_stamp - first.time_stamp; }


	static std::string to_string(uint64_t time) {
		const auto time_s  = double(time) / 1'000'000'000;
		const auto time_ms = double(time) / 1'000'000;
		const auto time_µs = double(time) / 1'000;
		const auto time_ns = double(time);

		std::ostringstream result;
		if (time >= 100'000'000) {
			result << time_s << 's';
		} else if (time >= 100'000) {
			result << time_ms << "ms";
		} else if (time >= 100) {
			result << time_µs << "µs";
		} else {
			result << time_ns << "ns";
		}
		return result.str();
	}
};

struct CodeSectionTimer {
	using TimeStampType = TimeStamp<const char *>;
	const TimeStampType start;

	explicit CodeSectionTimer(const char *name) : start(name) {}
	CodeSectionTimer(CodeSectionTimer &)  = delete;
	CodeSectionTimer(CodeSectionTimer &&) = delete;
	void operator=(CodeSectionTimer &)    = delete;

	~CodeSectionTimer() {
		const TimeStampType end("");

		std::cout << "Code section : " << start.name << " took "
				  << TimeStampType::to_string(TimeStampType::get_diff(start, end)) << std::endl;
	}
};

/**
 * @brief Prints the time passed between the start and the end of the code section.
 */
#define CODE_SECTION_TIMER_CONCATENATE(A, B) A##B
#define CODE_SECTION_TIMER_CONCATENATE2(A, B) CODE_SECTION_TIMER_CONCATENATE(A, B)
#define CODE_SECTION_TIMER                                                                                             \
	const auto CODE_SECTION_TIMER_CONCATENATE2(code_section_timer_internal_do_not_touch, __LINE__) =                   \
			CodeSectionTimer(__PRETTY_FUNCTION__)

/*
Timer class holds information on a measurement series, consisting of a number of events.
The measurements can then be logged to the console.
Events are named using the NAME_TYPE type. It can be one of int, std::string, const char*. Other types should work as well, but are not tested.
*/
template<class NAME_TYPE = int>
struct Timer {
	using TIME_STAMP_TYPE = TimeStamp<NAME_TYPE>;
	std::vector<TIME_STAMP_TYPE> time_stamps{};
	int                          id = 0;

	/*
    Initialize reference point from where the measurements start. Call only once.
    */
	void initialize() { add(); }

	/*
    Add a named event. Must be called after initialize.
     */
	const Timer &add(NAME_TYPE name) {
		time_stamps.emplace_back(name);
		return *this;
	}


	static constexpr const char *integer_string_literal_helper(int i) {
		const char *ints[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
		if (i < 10) return ints[i];
		return "";
	}

	/*
    Add an unnamed event. Must be called after initialize.
     */
	const Timer &add() {
		if constexpr (std::is_convertible<int, NAME_TYPE>::value) {
			add(id++);
		} else if constexpr (std::is_same<NAME_TYPE, std::string>::value) {
			add(std::to_string(id));
			++id;
		} else if constexpr (std::is_same<NAME_TYPE, const char *>::value) {
			add(integer_string_literal_helper(id));
			++id;
		} else {
			add({});
		}
		return *this;
	}

	[[nodiscard]] uint64_t get_time_since_init(int index) const {
		return TIME_STAMP_TYPE::get_diff(time_stamps[0], time_stamps[index]);
	}

	[[nodiscard]] uint64_t get_time_since_last(int index) const {
		return TIME_STAMP_TYPE::get_diff(time_stamps[index - 1], time_stamps[index]);
	}

	/*
    Print information about the last measurement.
    */
	void print_current() const {
		const int index = time_stamps.size() - 1;
		std::cout << "Timer : " << time_stamps[index].name << " after "
				  << TIME_STAMP_TYPE::to_string(get_time_since_last(index)) << " at "
				  << TIME_STAMP_TYPE::to_string(get_time_since_init(index)) << "\n";
	}

	/*
    Log all measurements
    */
	void log() const {
		const int length = time_stamps.size();
		std::cout << "Timer :\n";
		for (int i = 1; i < length; i++) {
			const auto time_since_last = TIME_STAMP_TYPE::to_string(get_time_since_last(i));
			const auto time_since_init = TIME_STAMP_TYPE::to_string(get_time_since_init(i));
			std::cout << "\t" << time_stamps[i].name << " after " << time_since_last << " at " << time_since_init
					  << "\n";
		}
	}
};
#endif
