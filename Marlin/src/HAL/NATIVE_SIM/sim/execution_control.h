#pragma once

#include <mutex>
#include <thread>
#include <functional>
#include <atomic>
#include <algorithm>
#include <cassert>

extern void setup();
extern void loop();
extern "C" void TIMER0_IRQHandler();
extern "C" void TIMER1_IRQHandler();

class KernelThread {
public:
  KernelThread(std::string name, void (*callback_loop)(),  void (*callback_init)() = nullptr) : name(name), thread_loop(callback_loop), thread_init(callback_init == nullptr ? callback_loop : callback_init), thread(&KernelThread::thread_main, this) {}
  void thread_main() {
    while (active) {
      if (execute_loop) {
        execute_loop = false;
        running = true;
        if (initilised) thread_loop(); else { thread_init(); initilised = true;}
        running = false;
      }
      std::this_thread::yield();
    }
  }

  uint64_t next_interrupt(const uint64_t main_frequency, const uint64_t ticks) {
    return timer_enabled ? timer_offset + (timer_compare * (main_frequency / (float)timer_rate)) : std::numeric_limits<uint64_t>::max();
  }

  std::atomic_bool execute_loop = false;
  std::atomic_bool active = true;
  std::atomic_bool running = false;

  bool initilised = false;
  bool timer_enabled = false;
  uint32_t timer_rate = 0;
  uint32_t timer_compare = 0;
  std::uint64_t timer_offset = 0;
  std::string name;

  std::function<void()> thread_loop;
  std::function<void()> thread_init;
  std::thread thread;
};

class Kernel {
public:
  // ordered highest priority first
  Kernel() : threads({KernelThread{"Stepper ISR", TIMER0_IRQHandler}, {"Temperature ISR", TIMER1_IRQHandler}, {"Marlin Loop", loop, setup}}) {}
  std::array<KernelThread, 3> threads;

  void kill() {
    for (auto& thread : threads) {
      thread.active = false;
      thread.thread.join();
    }
  }

  //execute highest priority thread with closest interrupt
  bool execute_loop() {
    uint64_t lowest = std::numeric_limits<uint64_t>::max();
    KernelThread* next = nullptr;
    for (auto& thread : threads) {
      auto value = thread.next_interrupt(frequency, ticks);
      if (value < lowest && thread.timer_enabled && std::this_thread::get_id() != thread.thread.get_id()) {
        lowest = value;
        next = &thread;
      }
    }
    if (next != nullptr) {
      next->execute_loop = true;
      while (next->running);
      next->timer_offset = ticks = lowest;
      return true;
    }
    //assert("this shouldnt happen? something must want to run");
    // but it does because of global constructors running Arduino code before any tasks are added to the simulation
    return false;
  }

  // if a thread wants to wait, see what should be executed during that wait
  inline void delayCycles(uint64_t cycles) {
    auto end = ticks + cycles;
    while (execute_loop() && ticks < end);
    ticks = end;
  }

  //Timers
  inline void timerInit(uint8_t thread_id, uint32_t rate) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_rate = rate;
      printf("Timer[%d] Initialised( rate: %d )\n", thread_id, rate);
    }
  }

  inline void timerStart(uint8_t thread_id, uint32_t interrupt_frequency) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_compare = threads[thread_id].timer_rate / interrupt_frequency;
      threads[thread_id].timer_offset = ticks;
      printf("Timer[%d] Started( frequency: %d )\n", thread_id, interrupt_frequency);
    }
  }

  inline void timerEnable(uint8_t thread_id) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_enabled = true;
      printf("Timer[%d] Enabled\n", thread_id);
    }
  }

  inline bool timerEnabled(uint8_t thread_id) {
    if (thread_id < threads.size())
      return threads[thread_id].timer_enabled;
    return false;
  }

  inline void timerDisable(uint8_t thread_id) {
    if (thread_id < threads.size())
      threads[thread_id].timer_enabled = false;
  }

  inline void timerSetCompare(uint8_t thread_id, uint64_t compare) {
    if (thread_id < threads.size())
      threads[thread_id].timer_compare = compare;
  }

  inline uint64_t timerGetCount(uint8_t thread_id) {
    if (thread_id < threads.size())
      return (ticks - threads[thread_id].timer_offset) * (threads[thread_id].timer_rate / float(frequency) );
    return 0;
  }

  inline uint64_t timerGetCompare(uint8_t thread_id) {
    if (thread_id < threads.size())
      return threads[thread_id].timer_compare;
    return 0;
  }

  // Clock
  inline uint64_t getTicks() {
    return ticks;
  }

  inline uint64_t nanosToTicks(uint64_t ns) {
    return ns * (frequency / 1000000);
  }

  inline uint64_t nanosToTicks(uint64_t ns, uint64_t freq) {
    return ns * (freq / 1000000);
  }

  inline uint64_t ticksToNanos(uint64_t tick) {
    return tick / (frequency / 1000000);
  }

  inline uint64_t ticksToNanos(uint64_t tick, uint64_t freq) {
    return tick / (freq / 1000000);
  }

  inline uint64_t nanos() {
    return ticksToNanos(ticks);
  }

  inline uint64_t micros() {
    return nanos() / 1000;
  }

  inline uint64_t millis() {
    return micros() / 1000000;
  }

  inline double seconds() {
    return nanos() / 1000000000.0;
  }

  inline void delayMicros(uint64_t micros) {
    delayCycles(micros * (frequency / 1000000));
  }

  inline void delayMillis(uint64_t millis) {
    delayCycles(millis * (frequency / 1000));
  }

  inline void delaySeconds(double secs) {
    delayCycles(secs * frequency);
  }

  std::atomic_uint64_t ticks = 0;
  static constexpr uint32_t frequency = 100000000;
};

extern Kernel kernel;
