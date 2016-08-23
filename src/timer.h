#ifndef TIMER__H
#define TIMER__H

#include <boost/container/vector.hpp>

#ifdef __COMPILE_AS_WINDOWS__
#include <Windows.h>
#elif defined (__COMPILE_AS_LINUX__)
#include <time.h>
#endif

#include "common.h"

class Timer {
public:
  Timer();
  // Starts the timer.
  void start();

  // Stops the timer.
  void stop();

  // Stops and resets the timer.
  void reset();

  // Gets the amount of elapsed clock cycles since start.
  int64_t get_elapsed_cycles();

  // Gets the elapsed time in [ns] since start.
  double get_elapsed_ns();

  // Gets the clock frequency of the CPU.
  int64_t get_clock_frequency();

private:
#ifdef __COMPILE_AS_WINDOWS__
  LARGE_INTEGER start_;
  LARGE_INTEGER end_;
  LARGE_INTEGER freq_;
#elif defined (__COMPILE_AS_LINUX__)
  timespec start_;
  timespec end_;
  
  timespec diff(timespec start, timespec end);  
#endif

};

class WindowTimer {
public:
  WindowTimer(uint8_t window_size = 0);

  // Starts the timer.
  void start();

  // Adds to the window average.
  void step();

  // Stops the timer.
  void stop();

  // Stops and resets the timer.
  void reset();

  // Gets the average amount of elapsed clock cycles since start.
  int64_t get_elapsed_cycles();

  // Gets the average elapsed time in [ns] since start.
  double get_elapsed_ns();

  // Gets the average amount of elapsed clock cycles since start.
  double get_avg_elapsed_cycles();

  // Gets the average elapsed time in [ns] since start.
  double get_avg_elapsed_ns();

  // Gets the clock frequency of the CPU.
  int64_t get_clock_frequency();

private:
#ifdef __COMPILE_AS_WINDOWS__
  LARGE_INTEGER start_;
  LARGE_INTEGER end_;
  LARGE_INTEGER freq_;
#elif defined (__COMPILE_AS_LINUX__)
  Timer timer_;
#endif

  uint8_t iterator_;
  uint8_t window_size_;
  boost::container::vector<int64_t> window_;
};

#endif
