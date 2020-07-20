#ifdef __PLAT_NATIVE_SIM__

#include "execution_control.h"

//execute highest priority thread with closest interrupt, return true if something was executed
bool Kernel::execute_loop( uint64_t max_end_ticks) {
  //call_stack.push_back(std::this_thread::get_id());
  uint64_t lowest = std::numeric_limits<uint64_t>::max();
  KernelThread* next = nullptr;
  for (auto& thread : threads) {
    uint64_t value = thread.next_interrupt(frequency);
    if (timers_active && value < lowest && value < max_end_ticks && thread.timer_enabled && this_thread != &thread) {
      lowest = value;
      next = &thread;
    }
  }
  if (next != nullptr) {
    auto current_ticks = ticks.load();

    // printf("[%ld]", ticksToNanos(current_ticks));
    // for (auto& thread_id : call_stack) {
    //   if (auto it = thread_lookup.find(thread_id); it != thread_lookup.end()) {
    //     printf(" <%s>", (char*)threads[it->second].name.c_str());
    //   } else {
    //     //printf(" <External[%lu]> ", std::hash<std::thread::id>{}(thread_id));
    //     printf(" <Kernel>");
    //   }
    // }

    // std::int64_t tick_diff = (std::int64_t)lowest - current_ticks;
    // if (this_thread != nullptr) {
    //   printf("<%s> -> Running: %s (%ld)\n",(char *)this_thread->name.c_str(), (char *)next->name.c_str(), tick_diff);
    // } else {
    //   printf("<KERNEL> -> Running: %s (%ld)\n", (char *)next->name.c_str(), tick_diff);
    // }

    // if (lowest > current_ticks) ticks = lowest; // this iterrupt is triggerig, so current ticks must be at least lowest

    if (ticks > lowest) next->timer_offset = ticks; // late interrupt
    else next->timer_offset = next->next_interrupt(frequency); // timer was reset when the interrupt fired
    ticks = next->timer_offset;

    auto old_thread = this_thread;
    this_thread = next;

    if (next->initilised) {
      next->thread_loop();  //time can pass here
    } else {
      next->initilised = true;
      next->thread_init();
    }

    this_thread = old_thread;

    //if(real_time_lock) wait_until_ticks_to_nanos time passes, yeild etx sleepy now
    //call_stack.pop_back();
    return true;
  }
  //call_stack.pop_back();
  return false;
}

// if a thread wants to wait, see what should be executed during that wait
void Kernel::delayCycles(uint64_t cycles) {
  auto end = ticks + cycles;
  while (execute_loop(end) && ticks < end);
  if (end > ticks) ticks = end;
}

// this was neede for when marlin loops idle waiting for an event with no delays
void Kernel::yield() {
  if (this_thread != nullptr) {
    auto max_yield = this_thread->next_interrupt(frequency);
    if(!execute_loop(max_yield)) { // dont wait longer than this threads exec period
      ticks = max_yield;
      this_thread->timer_offset = max_yield; // there was nothing to run, and we now overrun our next cycle.
    }
  } else {
    assert(false); // what else could have yielded?
    execute_loop();
  }
}


#endif // __PLAT_NATIVE_SIM__