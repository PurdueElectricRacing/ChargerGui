#ifndef __TIMER_H__
#define __TIMER_H__

#include <chrono>
#include <ctime>



using namespace std::chrono;

class Timer
{
public:
  Timer(uint64_t duration = 0) {
    this->duration = duration;
    start_time = steady_clock::now();
  };

  void start() {
    start_time = steady_clock::now();
    running = true;
  };

  bool expired() {
    if (running && elapsedMs() > duration)
    {
      return true;
    }
    return false;
  };

  uint64_t elapsedMs() {
    return duration_cast<milliseconds>(steady_clock::now() - start_time).count();
  }

  void run()
  {
    start();
    while (!expired())
    {

    }
  }

  steady_clock::time_point start_time;
  uint64_t duration;
  bool running = false;
};

#endif
