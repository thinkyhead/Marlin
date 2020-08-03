#ifdef __PLAT_NATIVE_SIM__

#include "user_interface.h"
#include "execution_control.h"


//execute highest priority thread with closest interrupt, return true if something was executed
bool Kernel::execute_loop( uint64_t max_end_ticks) {
  //todo: investigate dataloss when pulling from SerialMonitor rather than pushing from here

  if (usb_serial.transmit_buffer.available()) {
    char buffer[usb_serial.transmit_buffer_size];
    auto count = usb_serial.transmit_buffer.read((uint8_t *)buffer, usb_serial.transmit_buffer_size - 1);
    buffer[count] = '\0';
    std::dynamic_pointer_cast<SerialMonitor>(UserInterface::ui_elements["Serial Monitor"])->insert_text(buffer);
  }

  // make sure the timing mode changes at a controlled point in execution
  if (timing_mode_toggle != timing_mode_toggle_last && this_thread == nullptr) {
    timing_mode_toggle_last = timing_mode_toggle;
    if (timing_mode == TimingMode::REALTIME_SCALED) {
      timing_mode = TimingMode::ISRSTEP;
    } else if (timing_mode == TimingMode::ISRSTEP) {
      last_clock_read = clock.now();
      realtime_nanos = ticksToNanos(ticks.load());
      timing_mode = TimingMode::REALTIME_SCALED;
    }
  }

  uint64_t current_ticks = getTicks();

  /**
   *
   *  todo: call update on hardware that has registered it needs to trigger hardware interrupts here
   *
   * */


  // todo: move them back into the same array to get rid of this code duplication?..
  if (timing_mode == TimingMode::ISRSTEP) {
    uint64_t lowest_isr = std::numeric_limits<uint64_t>::max();
    KernelTimer* next_isr = nullptr;
    for (auto& timer : timers) {
      uint64_t value = timer.next_interrupt(frequency);
      if (timers_active && value < lowest_isr && value < max_end_ticks && timer.enabled()) {
        lowest_isr = value;
        next_isr = &timer;
      }
    }

    uint64_t lowest = std::numeric_limits<uint64_t>::max();
    KernelThread* next = nullptr;
    for (auto& thread : threads) {
      uint64_t value = thread.next_interrupt(frequency);
      if (timers_active && value < lowest && value < max_end_ticks && thread.timer_enabled && this_thread != &thread) {
        lowest = value;
        next = &thread;
      }
    }

    if (next_isr != nullptr && lowest_isr < lowest) {
      if (current_ticks > lowest_isr) {
        isr_timing_error =current_ticks - lowest_isr;
        next_isr->source_offset = current_ticks; // late interrupt
      } else {
        next_isr->source_offset = next_isr->next_interrupt(frequency); // timer was reset when the interrupt fired
        isr_timing_error = 0;
      }
      setTicks(next_isr->source_offset);
      next_isr->execute();
      return true;
    }

    if (next != nullptr) {
      if (current_ticks > lowest) next->timer_offset = current_ticks; // late interrupt
      else next->timer_offset = next->next_interrupt(frequency); // timer was reset when the interrupt fired
      setTicks(next->timer_offset);

      auto old_thread = this_thread;
      this_thread = next;
      next->execute();
      this_thread = old_thread;
      return true;
    }

  } else {

    for (auto& timer : timers) {
      if (timer.interrupt(current_ticks, frequency)) {
        isr_timing_error = ticksToNanos(current_ticks - timer.next_interrupt(frequency));
        timer.source_offset = current_ticks;
        timer.execute();
        return true;
      }
    }

    for (auto& thread : threads) {
      if (this_thread != &thread && thread.interrupt(current_ticks, frequency)) {
        thread.timer_offset = current_ticks;
        KernelThread* old_thread = this_thread;
        this_thread = &thread;
        thread.execute();
        this_thread = old_thread;
        return true;
      }
    }
  }

  return false;
}

// if a thread wants to wait, see what should be executed during that wait
void Kernel::delayCycles(uint64_t cycles) {
  auto end = getTicks() + cycles;

  if (timing_mode == TimingMode::ISRSTEP) {
    while (execute_loop(end) && getTicks() < end);
    if (end > getTicks()) setTicks(end);
  } else {
    while ( getTicks() < end) execute_loop();
  }
}

// this is needed for when marlin loops idle waiting for an event with no delays (syncronize)
void Kernel::yield() {
  if (timing_mode == TimingMode::ISRSTEP) {
    if (this_thread != nullptr) {
      auto max_yield = this_thread->next_interrupt(frequency);
      if(!execute_loop(max_yield)) { // dont wait longer than this threads exec period
        setTicks(max_yield);
        this_thread->timer_offset = max_yield; // there was nothing to run, and we now overrun our next cycle.
      }
    } else {
      assert(false); // what else could have yielded? ISRs should never yield
    }
  } else {
    execute_loop();
  }
}


#endif // __PLAT_NATIVE_SIM__
