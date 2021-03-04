#pragma once

#include <mutex>
#include <thread>
#include <functional>
#include <atomic>
#include <algorithm>
#include <cassert>
#include <map>
#include <sstream>

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

  bool interrupt(const uint64_t ticks, const uint64_t frequency) {
    return ticks > next_interrupt(frequency);
  }

  void execute() {
    if (!initilised) {
      initilised = true;
      thread_init();
    } else {
      thread_loop();
    }
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

struct KernelTimer {
  KernelTimer(std::string name, void (*callback)()) : name(name), isr_function(callback) {}

  bool interrupt(const uint64_t source_count, const uint64_t frequency) {
    return source_count > next_interrupt(frequency);
  }

  uint64_t next_interrupt(const uint64_t source_frequency) {
    return active ? source_offset + tickConvertFrequency(compare, timer_frequency, source_frequency) : std::numeric_limits<uint64_t>::max();
  }

  void initialise(const uint64_t timer_frequency) { this->timer_frequency = timer_frequency; }
  void start(const uint64_t source_count, const uint64_t interrupt_frequency) {
    compare = timer_frequency / interrupt_frequency;
    source_offset = source_count;
  }
  void enable() { active = true; }
  bool enabled() { return active; }
  void disable() { active = false; }

  // in timer frequency
  void set_compare(const uint64_t compare) { this->compare = compare; }
  uint64_t get_compare() { return compare; }
  uint64_t get_count(const uint64_t source_count, const uint64_t source_frequency) { return tickConvertFrequency(source_count - source_offset, source_frequency, timer_frequency); }


  void set_isr(std::string name, void (*callback)()) {
    isr_function = {callback};
    this->name = name;
  }
  void execute() {
    isr_function();
  }

  std::string name;
  bool active = false;
  uint64_t compare = 0, source_offset = 0, timer_frequency = 0;
  std::function<void()> isr_function;
};

class Kernel {
public:
  enum TimingMode {
    REALTIME_SCALED,
    ISRSTEP
    //REALTIME_SIGNAL
  };

  // ordered highest priority first
  Kernel() : threads({KernelThread{"Marlin Loop", loop, setup}}),
             timers({KernelTimer{"Stepper ISR", TIMER0_IRQHandler}, {"Temperate ISR", TIMER1_IRQHandler}}),
             last_clock_read(clock.now()) {
    threads[0].timer_offset = getTicks();
    threads[0].timer_rate = 1000000;
    threads[0].timer_compare = 500;
  }
  std::array<KernelThread, 1> threads;
  std::array<KernelTimer, 2> timers;
  //std::array<KernelTimer, N> gpio_interrupt;

  KernelThread* this_thread = nullptr;
  std::vector<KernelThread*> call_stack;
  bool timers_active = true;
  bool timing_mode_toggle = false;
  bool timing_mode_toggle_last = false;


  TimingMode timing_mode = TimingMode::REALTIME_SCALED;
  std::chrono::high_resolution_clock clock;
  std::chrono::high_resolution_clock::time_point last_clock_read;
  std::atomic_uint64_t isr_timing_error = 0;

  inline void updateRealtimeTicks() {
    auto now = clock.now();
    auto delta = now - last_clock_read;
    uint64_t delta_uint64 = std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count();
    if(delta_uint64 > std::numeric_limits<std::uint64_t>::max() - ONE_BILLION) {
      //printf("rt info: %ld : %f\n", delta_uint64, realtime_scale.load());
      //aparently time can go backwards, thread issue?
      delta_uint64 = 0;
    }
    uint64_t delta_uint64_scaled = delta_uint64 * realtime_scale;
    if (delta_uint64_scaled != 0) {
      last_clock_read = now;
      realtime_nanos += delta_uint64_scaled;
    }
    ticks.store(nanosToTicks(realtime_nanos));
  }

  //execute highest priority thread with closest interrupt, return true if something was executed
  bool execute_loop(uint64_t max_end_ticks = std::numeric_limits<uint64_t>::max());
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

  inline void timerInit(uint8_t timer_id, uint32_t rate) {
    if (timer_id < timers.size()) {
       timers[timer_id].timer_frequency = rate;
      // printf("Timer[%d] Initialised( rate: %d )\n", timer_id, rate);
    }
  }

  inline void timerStart(uint8_t timer_id, uint32_t interrupt_frequency) {
    if (timer_id < timers.size()) {
      timers[timer_id].compare = timers[timer_id].timer_frequency / interrupt_frequency;
      timers[timer_id].source_offset = getTicks();
      // printf("Timer[%d] Started( frequency: %d compare: %ld)\n", timer_id, interrupt_frequency, timers[timer_id].compare);
    }
  }

  inline void timerEnable(uint8_t timer_id) {
    if (timer_id < timers.size()) {
      timers[timer_id].active = true;
      // printf("Timer[%d] Enabled\n", timer_id);
    }
  }

  inline bool timerEnabled(uint8_t timer_id) {
    if (timer_id < timers.size())
      return timers[timer_id].active;
    return false;
  }

  inline void timerDisable(uint8_t timer_id) {
    if (timer_id < timers.size()) {
      timers[timer_id].active = false;
      //printf("Timer[%d] Disabled\n", timer_id);
    }
  }

  inline void timerSetCompare(uint8_t timer_id, uint64_t compare) {
    if (timer_id < timers.size()) {
      timers[timer_id].compare = compare;
    }
  }

  inline uint64_t timerGetCount(uint8_t timer_id) {
    if (timer_id < timers.size()) {
      if (timing_mode == TimingMode::ISRSTEP) {
        //in timeskip mode time must pass here for the stepper isr pulse counter (time + 100ns)
        setTicks(getTicks() + 1 + nanosToTicks(100, timers[timer_id].timer_frequency));
      }
      return timers[timer_id].get_count(getTicks(), frequency);
    }
    return 0;
  }

  inline uint64_t timerGetCompare(uint8_t timer_id) {
    if (timer_id < timers.size())
      return timers[timer_id].compare;
    return 0;
  }

  // Clock
  inline uint64_t getTicks() {
    if (timing_mode == TimingMode::REALTIME_SCALED) {
      updateRealtimeTicks();
    }
    return ticks.load();
  }

  inline void setTicks(uint64_t new_ticks) {
    if (timing_mode == TimingMode::ISRSTEP) {
      ticks.store(new_ticks);
    }
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
    if (timing_mode == TimingMode::ISRSTEP) {
      setTicks(getTicks() + 1 + nanosToTicks(100)); // Marlin has loops that only break after x ticks, so we need to increment ticks here
    }
    return ticksToNanos(getTicks());
  }

  inline uint64_t micros() {
    return nanos() / ONE_THOUSAND;
  }

  inline uint64_t millis() {
    return nanos() / ONE_MILLION;
  }

  inline double seconds() {
    return ticksToNanos(getTicks()) / double(ONE_BILLION);
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

  std::atomic<float> realtime_scale = 1.0;
  std::atomic_uint64_t ticks{0};
  uint64_t realtime_nanos = 0;
  static constexpr uint32_t frequency = 100'000'000;
};

extern Kernel kernel;
