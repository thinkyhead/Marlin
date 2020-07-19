/**
 * Marlin 3D Printer Firmware
 *
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifdef __PLAT_NATIVE_REALTIME__


extern void setup();
extern void loop();

#include <thread>
#include <pthread.h>

#include <iostream>
#include <fstream>

#include <src/inc/MarlinConfig.h>
#include <stdio.h>
#include <stdarg.h>
#include "../shared/Delay.h"

bool finished = false;
bool main_finished = false;

// simple stdout / stdin implementation for fake serial port
void write_serial_thread() {
  char buffer[129] = {};
  while (!main_finished) {
    if (usb_serial.transmit_buffer.available()) {
      auto count = usb_serial.transmit_buffer.read((uint8_t*)buffer, usb_serial.transmit_buffer.available());
      buffer[count] = '\0';
      fputs(buffer, stdout);
    }
    DELAY_US(100);
    std::this_thread::yield();
  }
}

void read_serial_thread() {
  char buffer[255] = {};
  while (!main_finished) {
    std::size_t len = _MIN(usb_serial.receive_buffer.free(), 254U);
    if (fgets(buffer, len, stdin))
        usb_serial.receive_buffer.write((uint8_t*)buffer, strlen(buffer));
    DELAY_US(100);
    std::this_thread::yield();
  }
}

int main() {
  sched_param sch;
  int policy;
  pthread_getschedparam(pthread_self(), &policy, &sch);
  sch.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch)) {
      std::cout << "Unable to change thread scheduler priority (" << std::strerror(errno) << ")\n";
  }

  std::thread write_serial (write_serial_thread);
  std::thread read_serial (read_serial_thread);

  #if NUM_SERIAL > 0
    MYSERIAL0.begin(BAUDRATE);
    SERIAL_FLUSHTX();
  #endif

  Clock::setFrequency(F_CPU);

  HAL_timer_init();

  DELAY_US(10000);
  setup();
  while (!finished) {
    loop();
    std::this_thread::yield();
  }

  // signal that main is finished and the other threads can now exit safely
  main_finished = true;

  write_serial.join();
  read_serial.join();

}

#endif // __PLAT_NATIVE_REALTIME__
