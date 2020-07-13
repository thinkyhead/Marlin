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
#ifdef __PLAT_LINUX__

#define ENABLE_SIMULATION
//#define GPIO_LOGGING // Full GPIO Logging
//#define POSITION_LOGGING // Positional Logging

extern void setup();
extern void loop();

#include <thread>
#include <pthread.h>

#include <iostream>
#include <fstream>

#include "../../inc/MarlinConfig.h"
#include <stdio.h>
#include <stdarg.h>
#include "../shared/Delay.h"

#ifdef ENABLE_SIMULATION
#include "hardware/IOLoggerCSV.h"
#include "hardware/Heater.h"
#include "hardware/LinearAxis.h"
#include "hardware/ST7920Device.h"
#endif

bool finished = false;

// simple stdout / stdin implementation for fake serial port
void write_serial_thread() {
  while (!finished) {
    for (std::size_t i = usb_serial.transmit_buffer.available(); i > 0; i--) {
      fputc(usb_serial.transmit_buffer.read(), stdout);
    }
    std::this_thread::yield();
  }
}

void read_serial_thread() {
  char buffer[255] = {};
  while (!finished) {
    std::size_t len = _MIN(usb_serial.receive_buffer.free(), 254U);
    if (fgets(buffer, len, stdin))
      for (std::size_t i = 0; i < strlen(buffer); i++)
        usb_serial.receive_buffer.write(buffer[i]);
    std::this_thread::yield();
  }
}

#ifdef ENABLE_SIMULATION
void simulation_loop() {
  Heater hotend(HEATER_0_PIN, TEMP_0_PIN);
  Heater bed(HEATER_BED_PIN, TEMP_BED_PIN);
  LinearAxis x_axis(X_ENABLE_PIN, X_DIR_PIN, X_STEP_PIN, X_MIN_PIN, X_MAX_PIN);
  LinearAxis y_axis(Y_ENABLE_PIN, Y_DIR_PIN, Y_STEP_PIN, Y_MIN_PIN, Y_MAX_PIN);
  LinearAxis z_axis(Z_ENABLE_PIN, Z_DIR_PIN, Z_STEP_PIN, Z_MIN_PIN, Z_MAX_PIN);
  LinearAxis extruder0(E0_ENABLE_PIN, E0_DIR_PIN, E0_STEP_PIN, P_NC, P_NC);
  ST7920Device display(LCD_PINS_D4, LCD_PINS_ENABLE, LCD_PINS_RS, BEEPER_PIN, BTN_EN1, BTN_EN2, BTN_ENC, KILL_PIN);

  #ifdef GPIO_LOGGING
    IOLoggerCSV logger("all_gpio_log.csv");
    Gpio::attachLogger(&logger);
  #endif
  #ifdef POSITION_LOGGING
    std::ofstream position_log;
    position_log.open("axis_position_log.csv");
    int32_t x = 0, y = 0, z = 0;
  #endif

  while (!finished) {

    hotend.update();
    bed.update();

    x_axis.update();
    y_axis.update();
    z_axis.update();
    extruder0.update();
    display.update();
    finished = display.close_request;

    #ifdef POSITION_LOGGING
      if (x_axis.position != x || y_axis.position != y || z_axis.position != z) {
        uint64_t update = std::max({x_axis.last_update, y_axis.last_update, z_axis.last_update});
        position_log << update << ", " << x_axis.position << ", " << y_axis.position << ", " << z_axis.position << std::endl;
        position_log.flush();
        x = x_axis.position;
        y = y_axis.position;
        z = z_axis.position;
      }
    #endif
    #ifdef GPIO_LOGGING
      // flush the logger
      logger.flush();
    #endif

    std::this_thread::yield();
  }
}
#endif

int main() {

  sched_param sch;
  int policy;
  pthread_getschedparam(pthread_self(), &policy, &sch);
  sch.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch)) {
      std::cout << "Unable to increase thread priority: " << std::strerror(errno) << '\n';
  }

  std::thread write_serial (write_serial_thread);
  std::thread read_serial (read_serial_thread);

  #if NUM_SERIAL > 0
    MYSERIAL0.begin(BAUDRATE);
    SERIAL_ECHOLNPGM("x86_64 Initialized");
    SERIAL_FLUSHTX();
  #endif

  Clock::setFrequency(F_CPU);
  Clock::setTimeMultiplier(1.0); // some testing at 10x

  HAL_timer_init();

  #ifdef ENABLE_SIMULATION
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
  }
  std::thread simulation (simulation_loop);
  #endif

  DELAY_US(10000);
  setup();
  while (!finished) {
    loop();
    std::this_thread::yield();
  }

  #ifdef ENABLE_SIMULATION
  simulation.join();
  SDL_Quit();
  #endif

  write_serial.join();
  read_serial.join();
}

#endif // __PLAT_LINUX__
