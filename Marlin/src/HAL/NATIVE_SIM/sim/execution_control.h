#pragma once

#include <mutex>
#include <thread>
#include <functional>
#include <atomic>
#include <algorithm>
#include <cassert>
#include <map>

extern void setup();
extern void loop();
extern "C" void TIMER0_IRQHandler();
extern "C" void TIMER1_IRQHandler();


constexpr inline uint64_t tickConvertFrequency(std::uint64_t value, std::uint64_t from, std::uint64_t to) {
  return from > to ? value / (from / to) : value * (to / from);
}

class KernelThread {
public:
  KernelThread(std::string name, void (*callback_loop)(),  void (*callback_init)() = nullptr) : name(name), thread_loop(callback_loop), thread_init(callback_init == nullptr ? callback_loop : callback_init) {}

  uint64_t next_interrupt(const uint64_t main_frequency) {
    return timer_enabled ? timer_offset + tickConvertFrequency(timer_compare, timer_rate, main_frequency) : std::numeric_limits<uint64_t>::max();
  }

  uint64_t timer_count(const uint64_t system_ticks, const uint64_t system_frequency) {
    return tickConvertFrequency(system_ticks - timer_offset, timer_rate, system_frequency);
  }

  bool initilised = false;
  bool timer_enabled = false;
  std::uint64_t timer_rate{0};
  std::uint64_t timer_compare{0};
  std::uint64_t timer_offset{0};
  std::string name;

  std::function<void()> thread_loop;
  std::function<void()> thread_init;
};

class Kernel {
public:
  // ordered highest priority first
  Kernel() : threads({KernelThread{"Stepper ISR", TIMER0_IRQHandler}, {"Temperature ISR", TIMER1_IRQHandler}, {"Marlin Loop", loop, setup}}) {}
  std::array<KernelThread, 3> threads;
  KernelThread* this_thread = nullptr;
  bool timers_active = true;

  //execute highest priority thread with closest interrupt, return true if something was executed
  bool execute_loop( uint64_t max_end_ticks = std::numeric_limits<uint64_t>::max());
  // if a thread wants to wait, see what should be executed during that wait
  void delayCycles(uint64_t cycles);
  // this was neede for when marlin loops idle waiting for an event with no delays
  void yield();

  //Timers
  inline void disableInterrupts() {
    timers_active = false;
  }

  inline void enableInterrupts() {
    timers_active = true;
  }

  inline void timerInit(uint8_t thread_id, uint32_t rate) {
    if (thread_id < threads.size()) {
       threads[thread_id].timer_rate = rate;
      // printf("Timer[%d] Initialised( rate: %d )\n", thread_id, rate);
    }
  }

  inline void timerStart(uint8_t thread_id, uint32_t interrupt_frequency) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_compare = threads[thread_id].timer_rate / interrupt_frequency;
      threads[thread_id].timer_offset = ticks.load();
      // printf("Timer[%d] Started( frequency: %d compare: %ld)\n", thread_id, interrupt_frequency, threads[thread_id].timer_compare);
    }
  }

  inline void timerEnable(uint8_t thread_id) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_enabled = true;
      // printf("Timer[%d] Enabled\n", thread_id);
    }
  }

  inline bool timerEnabled(uint8_t thread_id) {
    if (thread_id < threads.size())
      return threads[thread_id].timer_enabled;
    return false;
  }

  inline void timerDisable(uint8_t thread_id) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_enabled = false;
      //printf("Timer[%d] Disabled\n", thread_id);
    }
  }

  inline void timerSetCompare(uint8_t thread_id, uint64_t compare) {
    if (thread_id < threads.size()) {
      threads[thread_id].timer_compare = compare;
    }
  }

  inline uint64_t timerGetCount(uint8_t thread_id) {
    if (thread_id < threads.size()) {
      ticks.fetch_add(1 + nanosToTicks(100, threads[thread_id].timer_rate)); // todo: realtime control? time must pass here fore the stepper isr pulse counter
      return threads[thread_id].timer_count(ticks.load(), frequency);
    }
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

  // constants to reduce risk of typos
  static constexpr uint64_t ONE_BILLION  = 1000'000'000;
  static constexpr uint64_t ONE_MILLION  = 1000'000;
  static constexpr uint64_t ONE_THOUSAND = 1000;

  constexpr static uint64_t nanosToTicks(const uint64_t value, const uint64_t freq) {
    return freq > ONE_BILLION ? value * (freq / ONE_BILLION) : value / (ONE_BILLION / freq);
  }

  inline uint64_t nanosToTicks(uint64_t value) {
    return nanosToTicks(value, frequency);
  }

  constexpr static uint64_t ticksToNanos(const uint64_t value, const uint64_t freq) {
    return freq > ONE_BILLION ? value / (freq / ONE_BILLION) : value * (ONE_BILLION / freq);
  }

  inline uint64_t ticksToNanos(uint64_t value) {
    return ticksToNanos(value, frequency);
  }

  inline uint64_t nanos() {
    ticks.fetch_add(nanosToTicks(100)); // todo: some things loop on a delay until the expected period has passed, increase ticks in an interupt at clock frequency?
    return ticksToNanos(ticks);
  }

  inline uint64_t micros() {
    return nanos() / ONE_THOUSAND;
  }

  inline uint64_t millis() {
    return nanos() / ONE_MILLION;
  }

  inline double seconds() {
    return ticksToNanos(ticks.load()) / double(ONE_BILLION);
  }

  inline void delayNanos(uint64_t ns) {
    delayCycles(nanosToTicks(ns));
  }

  inline void delayMicros(uint64_t micros) {
    delayCycles(nanosToTicks(micros * ONE_THOUSAND));
  }

  inline void delayMillis(uint64_t millis) {
    delayCycles(nanosToTicks(millis * ONE_MILLION));
  }

  inline void delaySeconds(double secs) {
    delayCycles(nanosToTicks(secs * ONE_BILLION));
  }

  std::atomic_uint64_t ticks{0};
  static constexpr uint32_t frequency = 100'000'000;
};

extern Kernel kernel;
