#include "timer.h"

double NS_TO_SEC = 1e9;

Timer::Timer() {
	reset();
	get_clock_frequency();
}

void Timer::start() {
#ifdef __COMPILE_AS_WINDOWS__
	QueryPerformanceCounter(&start_);
#elif defined (__COMPILE_AS_LINUX__)
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_);
#endif
}

void Timer::stop() {
#ifdef __COMPILE_AS_WINDOWS__
	QueryPerformanceCounter(&end_);
#elif defined (__COMPILE_AS_LINUX__)
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_);
#endif
}

double Timer::get_elapsed_ns() {
#ifdef __COMPILE_AS_WINDOWS__
	return (get_elapsed_cycles() * NS_TO_SEC) / freq_.QuadPart;
#elif defined (__COMPILE_AS_LINUX__)
	timespec tmp = diff(start_, end_);
	return (NS_TO_SEC * tmp.tv_sec) + tmp.tv_nsec;
#endif
}

int64_t Timer::get_elapsed_cycles() {
#ifdef __COMPILE_AS_WINDOWS__
	return end_.QuadPart - start_.QuadPart;
#elif defined (__COMPILE_AS_LINUX__)
	return 0;
#endif
}

#ifdef __COMPILE_AS_LINUX__
timespec Timer::diff(timespec start, timespec end) {
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
#endif

int64_t Timer::get_clock_frequency() {
#ifdef __COMPILE_AS_WINDOWS__
	if (!freq_.QuadPart) {
		QueryPerformanceFrequency(&freq_);
	}
	return freq_.QuadPart;
#elif defined (__COMPILE_AS_LINUX__)
	return 0;
#endif
}

void Timer::reset() {
#ifdef __COMPILE_AS_WINDOWS__
	start_ = end_ = LARGE_INTEGER();
#elif defined (__COMPILE_AS_LINUX__)
	start_.tv_sec = end_.tv_sec = 0;
	start_.tv_nsec = end_.tv_nsec = 0;
#endif
}

WindowTimer::WindowTimer(uint8_t window_size) : iterator_(0) {
	window_size_ = std::max(window_size, (uint8_t)1);
	window_.reserve(window_size_);
	for (uint8_t i = 0; i < window_size_; ++i) {
		window_.push_back(0);
	}
	reset();
	get_clock_frequency();
}

// Starts the timer.
void WindowTimer::start() {
#ifdef __COMPILE_AS_WINDOWS__
	QueryPerformanceCounter(&start_);
#elif (defined __COMPILE_AS_LINUX__)
	timer_.start();
#endif
}

// Starts the timer.
void WindowTimer::step() {
	window_[iterator_++] = get_elapsed_ns();
	iterator_ = iterator_ % window_size_;
}

// Stops the timer.
void WindowTimer::stop() {
#ifdef __COMPILE_AS_WINDOWS__
	QueryPerformanceCounter(&end_);
#elif (defined __COMPILE_AS_LINUX__)
	timer_.stop();
#endif
}

// Stops and resets the timer.
void WindowTimer::reset() {
#ifdef __COMPILE_AS_WINDOWS__
	start_ = end_ = LARGE_INTEGER();
#elif (defined __COMPILE_AS_LINUX__)
	timer_.reset();
#endif
	for (uint8_t i = 0; i < window_size_; ++i) {
		window_[i] = 0;
	}
}

// Gets the average amount of elapsed clock cycles since start.
int64_t WindowTimer::get_elapsed_cycles() {
#ifdef __COMPILE_AS_WINDOWS__
	return end_.QuadPart - start_.QuadPart;
#elif (defined __COMPILE_AS_LINUX__)
	return timer_.get_elapsed_cycles();
#endif
}

// Gets the average elapsed time in [ns] since start.
double WindowTimer::get_elapsed_ns() {
#ifdef __COMPILE_AS_WINDOWS__
	return (get_elapsed_cycles() * NS_TO_SEC) / freq_.QuadPart;
#elif defined (__COMPILE_AS_LINUX__)
	return timer_.get_elapsed_ns();
#endif
}

// double the average amount of elapsed clock cycles since start.
double WindowTimer::get_avg_elapsed_cycles() {
	double accum = 0;
	for (auto& n : window_) {
		accum += n;
	}
	return accum / window_size_;
}

// Gets the average elapsed time in [ns] since start.
double WindowTimer::get_avg_elapsed_ns() {
	double accum = 0;
	for (auto& n : window_) {
		accum += n;
	}
	return accum / window_size_;
}

// Gets the clock frequency of the CPU.
int64_t WindowTimer::get_clock_frequency() {
#ifdef __COMPILE_AS_WINDOWS__
	if (!freq_.QuadPart) {
		QueryPerformanceFrequency(&freq_);
	}
	return freq_.QuadPart;
#elif (defined __COMPILE_AS_LINUX__)
	return 0;
#endif
}
