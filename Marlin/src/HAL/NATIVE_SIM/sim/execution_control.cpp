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

  /**
   *
   *  call update on hardware that has registered it needs to trigger hardware interrupts here
   *
   * */

  for (auto& timer : timers) {
    if (timer.interrupt(getTicks(), frequency)) {
      timer.source_offset = getTicks();
      timer.execute();
      return true;
    }
  }

  for (auto& thread : threads) {
    if (this_thread != &thread && thread.interrupt(getTicks(), frequency)) {
      thread.timer_offset = getTicks();
      KernelThread* old_thread = this_thread;
      this_thread = &thread;
      thread.execute();
      this_thread = old_thread;
      return true;
    }
  }

  // uint64_t lowest = std::numeric_limits<uint64_t>::max();
  // KernelThread* next = nullptr;
  // for (auto& thread : threads) {
  //   uint64_t value = thread.next_interrupt(frequency);
  //   if (timers_active && value < lowest && value < max_end_ticks && thread.timer_enabled && this_thread != &thread && getTicks() > value) {
  //     lowest = value;
  //     next = &thread;
  //   }
  // }

  // if (next != nullptr) {
  //   call_stack.push_back(next);
  //   // for (auto t : call_stack) {
  //   //   printf("> [%s] \n", (char*)(t)->name.c_str());
  //   // }


  //   if (getTicks() > lowest) next->timer_offset = getTicks(); // late interrupt
  //   else next->timer_offset = next->next_interrupt(frequency); // timer was reset when the interrupt fired

  //   setTicks(next->timer_offset);

  //   auto old_thread = this_thread;
  //   this_thread = next;

  //   if (next->initilised) {
  //     next->thread_loop();  //time can pass here
  //   } else {
  //     next->initilised = true;
  //     next->thread_init();  //time can pass here //todo: segfault in function class in release build
  //   }

  //   this_thread = old_thread;
  //   call_stack.pop_back();
  //   return true;
  // }
  return false;
}

// if a thread wants to wait, see what should be executed during that wait
void Kernel::delayCycles(uint64_t cycles) {
  auto end = getTicks() + cycles;
  while (execute_loop(end) && getTicks() < end);
  if (end > getTicks()) setTicks(end);
}

// this was neede for when marlin loops idle waiting for an event with no delays
void Kernel::yield() {
  // if (this_thread != nullptr) {
  //   auto max_yield = this_thread->next_interrupt(frequency);
  //   if(!execute_loop(max_yield)) { // dont wait longer than this threads exec period
  //     setTicks(max_yield);
  //     this_thread->timer_offset = max_yield; // there was nothing to run, and we now overrun our next cycle.
  //   }
  // } else {
  //   assert(false); // what else could have yielded?
    execute_loop();
  // }
}


#endif // __PLAT_NATIVE_SIM__