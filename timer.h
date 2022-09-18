#ifndef TIMER_H
#define TIMER_H

#if __cplusplus < 201703L
#error "Use C++17 implementation"
#endif

#include <chrono>
#include <cstdint>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

/**
 * Disable the use of thread safe code using "#define DISABLE_TIMER_THREADS". DO NOT USE THIS IN PRODUCTION CODE!
 * Q&A:
 * 		Q: Should the user or the library be responsible for thread safety?
 * 		A: The library should be responsible for thread safety. But there is a switch to disable thread safety.
 *
 * 		Q: Do the logging functions need thread safety?
 * 		A: No, as the main idea is to call log in the end of the program/parallel section.
 * 		   Also logging does not crash the program. The user can still sync its on his own.
 * 		   This may change in the future, when we expand the logging functionality.
 */
#ifndef DISABLE_TIMER_THREADS
/**
 * Marks code to be required for thread safety.
 * This still requires #include \<thread\> but does not require it's functionality.
 */
#define TIMER_THREADS
#endif


/**
 * Debug mode:
 * 		This checks valid usage of the interfaces.
 * 		Enabled by "#define TIMER_DEBUG".
 * 		This can enable the use of assertions.
 * 		May be useful in early stages of development, or when errors are encountered.
 */
#ifdef TIMER_DEBUG

#include <csignal>
/*
 * I'm grateful if someone improves this.
 */
void print_stacktrace_and_exit() {
	raise(SIGSEGV);
	std::terminate();
}

/*
 * The key (and for now only) feature of this extension, is the (currently non generic) DebugStateTracker.
 * It Tracks the state of the program, and can be used to detect errors.
 * In theory, a state tracker tracks a state representation(bool or int) and a state name.
 * There are 2 function types for each state:
 * 		- update: changes the state
 * 		- check: checks if the state is as expected
 */
struct DebugStateTracker {
	bool initialized      = false;
	int  number_of_events = 0;

	/*
	 * Check for initialization.
	 */

	// state update function
	void debug_init() {
		if (initialized) {
			std::cerr << "DebugStateTracker::init() called twice" << std::endl;
			print_stacktrace_and_exit();
		}
		initialized = true;
	}

	// state check function
	void debug_check_if_initialized() const {
		if (!initialized) {
			std::cerr << "add() called before init()" << std::endl;
			print_stacktrace_and_exit();
		}
	}

	/*
	 * Check for sufficient number of events.
	 */

	// state update function
	void debug_add_event() { number_of_events++; }

	// state check function
	void debug_check_if_loggable() const {
		if (number_of_events <= 1) { // First one marks initialization
			std::cerr << "print_current() called without ever adding events" << std::endl;
			print_stacktrace_and_exit();
		}
	}
};

#else
struct DebugStateTracker {
	void debug_init() const { /* no-op */
	}

	void debug_check_if_initialized() const { /* no-op */
	}

	void debug_add_event() const { /* no-op */
	}

	void debug_check_if_loggable() const { /* no-op */
	}
};
#endif

/**
 *
 * Quick and dirty timing tool.
 *
 */

/*
 *TODO:
 *    - loop sections
 *    - printing formats
 *    - multithreaded flow graph
 */


inline uint64_t get_time_ns() {
	using namespace std::chrono;
	return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

template<class NAME_TYPE = int>
struct TimeStamp {
	const NAME_TYPE name;
	const uint64_t  time_stamp = get_time_ns();
#ifdef TIMER_THREADS
	const std::thread::id thread_id;

	explicit TimeStamp(NAME_TYPE id, std::thread::id thread_id) : name(id), thread_id(thread_id) {}
	explicit TimeStamp(NAME_TYPE id) : name(id), thread_id(std::this_thread::get_id()) {}
#else
	explicit TimeStamp(NAME_TYPE id) : name(id) {}
#endif

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

#define CODE_SECTION_TIMER_CONCATENATE(A, B) A##B
#define CODE_SECTION_TIMER_CONCATENATE2(A, B) CODE_SECTION_TIMER_CONCATENATE(A, B)
/**
 * @brief Prints the time passed between the start and the end of the code section.
 */
#define CODE_SECTION_TIMER                                                                                             \
	const auto CODE_SECTION_TIMER_CONCATENATE2(code_section_timer_internal_do_not_touch, __LINE__) =                   \
			CodeSectionTimer(__PRETTY_FUNCTION__)

/**
 * Timer class holds information on a measurement series, consisting of a number of events.
 * The measurements can then be logged to the console.
 * @tparam NAME_TYPE The type of the name of the event. Events are named using the NAME_TYPE type.
 * It can be one of int, std::string, const char*. Other types should work as well, but are not tested.
*/
template<class NAME_TYPE = int>
struct Timer : protected DebugStateTracker {
	using TIME_STAMP_TYPE = TimeStamp<NAME_TYPE>;
	std::vector<TIME_STAMP_TYPE> time_stamps{};

	// IDs for automatic naming
	int id = 0;

#ifdef TIMER_THREADS
	std::set<std::thread::id> thread_ids{};
	std::mutex                multithreading_guard{};
#endif

	/**
	 * Initialize reference point from where the measurements start. Call only once.
	 */
	void initialize() {
		debug_init();
		add();
	}

	void add_thread_unsafe(NAME_TYPE name) {
		debug_check_if_initialized();
		debug_add_event();
#ifdef TIMER_THREADS
		const auto thread_id = std::this_thread::get_id();
		thread_ids.insert(thread_id);
		time_stamps.emplace_back(name, thread_id);
#else
		time_stamps.emplace_back(name);
#endif
	}

	/**
	 * Add a named event. Must be called after initialize.
	 */
	const Timer &add(NAME_TYPE name) {
		/*
		 * This optimization is not tested well.
		 * The Idea is to disable multithreading dynamically, if the user does not use it.
		 * It could happen, that there is a race condition, on the first call if add().
		 */
		if (has_threads()) {
#ifdef TIMER_THREADS // has_threads() returns always false if !TIMER_THREADS
			std::lock_guard<std::mutex> lock(multithreading_guard);
			add_thread_unsafe(name);
#endif
		} else {
			add_thread_unsafe(name);
		}
		return *this;
	}

	/*
	 * Note this naming scheme for each thread is not at all guaranteed to be equal to other names.
	 * We name all threads which ever called add() from 0 to n-1.
	 */
	[[nodiscard]] int get_thread_name(const std::thread::id &id_) const {
#ifdef TIMER_THREADS
		int i = 0;
		for (auto &thread_id: thread_ids) {
			if (thread_id == id_) { return i; }
			i++;
		}
		return -1;
#else
		return 0;
#endif
	}

	[[nodiscard]] bool has_threads() const {
#ifdef TIMER_THREADS
		return thread_ids.size() > 1;
#else
		return false;
#endif
	}

	static constexpr const char *integer_string_literal_helper(int i) {
		const char *ints[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
		if (i < 10) return ints[i];
		return "";
	}

	/**
	 * Add an unnamed event. Must be called after initialize.
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

	[[nodiscard]] std::string thread_output_formatter(const std::thread::id id_) const {
		if (!has_threads()) { return ""; }
		static std::string result;
		result = " in thread : " + std::to_string(get_thread_name(id_));
		return result;
	}

	/**
	 * Print information about the last measurement.
	 */
	void print_current() const {
		debug_check_if_loggable();
		const int index = time_stamps.size() - 1;
		std::cout << "Timer : " << time_stamps[index].name << " after "
				  << TIME_STAMP_TYPE::to_string(get_time_since_last(index)) << " at "
				  << TIME_STAMP_TYPE::to_string(get_time_since_init(index))
				  << thread_output_formatter(time_stamps[index].thread_id) << "\n";
	}

	/**
	 * Log all measurements
	 */
	void log() const {
		const int length = time_stamps.size();
		std::cout << "Timer :\n";
		for (int i = 1; i < length; i++) {
			const auto time_since_last = TIME_STAMP_TYPE::to_string(get_time_since_last(i));
			const auto time_since_init = TIME_STAMP_TYPE::to_string(get_time_since_init(i));
			std::cout << "\t" << time_stamps[i].name << " after " << time_since_last << " at " << time_since_init
					  << thread_output_formatter(time_stamps[i].thread_id) << "\n";
		}
	}
};
#endif
