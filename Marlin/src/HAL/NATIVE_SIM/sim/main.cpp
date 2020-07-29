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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifdef __PLAT_NATIVE_SIM__

#include <thread>
#include <atomic>

#include "application.h"
#include "execution_control.h"

#include "src/inc/MarlinConfig.h"


std::atomic_bool main_finished = false;
Kernel kernel;

void HAL_idletask() {
  kernel.yield();
}

void marlin_main() {
  #if NUM_SERIAL > 0
    MYSERIAL0.begin(BAUDRATE);
    SERIAL_FLUSHTX();
  #endif

  //kernel.setFrequency(F_CPU);
  HAL_timer_init();
  kernel.threads[0].timer_enabled = true;
  while(!main_finished) {
    kernel.execute_loop();
    std::this_thread::yield();
  }
}

// Main code
int main(int, char**) {
  Application app;

  std::thread marlin_loop(marlin_main);

  while (app.active) {
    app.update();
    app.render();
    std::this_thread::yield();
  }

  main_finished = true;
  marlin_loop.join();

  return 0;
}

#endif // __PLAT_NATIVE_SIM__
