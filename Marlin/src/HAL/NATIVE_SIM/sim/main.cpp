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

//#define GPIO_LOGGING // Full GPIO Logging
//#define POSITION_LOGGING // Positional Logging

extern void setup();
extern void loop();

#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <chrono>
using namespace std::chrono_literals;


#include "execution_control.h"

#include "hardware/IOLoggerCSV.h"
#include "hardware/Heater.h"
#include "hardware/LinearAxis.h"
#include "hardware/ST7920Device.h"
#include "visualisation.h"

#include "src/inc/MarlinConfig.h"

bool finished = false;
bool main_finished = false;

std::atomic_bool initialised = false;
Kernel kernel;

// simple stdout / stdin implementation for fake serial port
void write_serial_thread() {
  char buffer[129] = {};
  while (!main_finished) {
    if (usb_serial.transmit_buffer.available()) {
      auto count = usb_serial.transmit_buffer.read((uint8_t*)buffer, usb_serial.transmit_buffer.available());
      buffer[count] = '\0';
      fputs(buffer, stdout);
    }
    std::this_thread::yield();
  }
}

void read_serial_thread() {
  char buffer[255] = {};
  while (!main_finished) {
    std::size_t len = _MIN(usb_serial.receive_buffer.free(), 254U);
    if (fgets(buffer, len, stdin))
        usb_serial.receive_buffer.write((uint8_t*)buffer, strlen(buffer));
    std::this_thread::yield();
  }
}

void simulation_loop() {
  Heater hotend(HEATER_0_PIN, TEMP_0_PIN);
  Heater bed(HEATER_BED_PIN, TEMP_BED_PIN);
  LinearAxis x_axis(X_ENABLE_PIN, X_DIR_PIN, X_STEP_PIN, X_MIN_PIN, X_MAX_PIN);
  LinearAxis y_axis(Y_ENABLE_PIN, Y_DIR_PIN, Y_STEP_PIN, Y_MIN_PIN, Y_MAX_PIN, true);
  LinearAxis z_axis(Z_ENABLE_PIN, Z_DIR_PIN, Z_STEP_PIN, Z_MIN_PIN, Z_MAX_PIN);
  LinearAxis extruder0(E0_ENABLE_PIN, E0_DIR_PIN, E0_STEP_PIN, P_NC, P_NC);
  ST7920Device display(LCD_PINS_D4, LCD_PINS_ENABLE, LCD_PINS_RS, BEEPER_PIN, BTN_EN1, BTN_EN2, BTN_ENC, KILL_PIN);
  Visualisation vis;

  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
    printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
  }

  vis.create(1280, 768);
  display.window_create(6);

  #ifdef GPIO_LOGGING
    IOLoggerCSV logger("all_gpio_log.csv");
    Gpio::attachLogger(&logger);
  #endif
  #ifdef POSITION_LOGGING
    std::ofstream position_log;
    position_log.open("axis_position_log.csv");
    int32_t x = 0, y = 0, z = 0;
  #endif

  vis.set_data_source(&x_axis.position, &y_axis.position,  &z_axis.position, &extruder0.position);

  hotend.update();
  bed.update();

  x_axis.update();
  y_axis.update();
  z_axis.update();
  extruder0.update();

  initialised = true;

  while (!main_finished) {

    hotend.update();
    bed.update();

    x_axis.update();
    y_axis.update();
    z_axis.update();
    extruder0.update();

    SDL_Event e;
    while( SDL_PollEvent( &e ) != 0 ) {
      if( e.type == SDL_QUIT ) {
        finished = true;
      }
      if( e.type == SDL_WINDOWEVENT) {
        if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
          finished = true;
        }
      }
      display.process_event(e);
      vis.process_event(e);
    }

    display.update();
    vis.update();


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
  vis.destroy();

  SDL_Quit();
}

int main() {
  std::thread write_serial (write_serial_thread);
  std::thread read_serial (read_serial_thread);

  #if NUM_SERIAL > 0
    MYSERIAL0.begin(BAUDRATE);
    SERIAL_FLUSHTX();
  #endif

  // kernel.setFrequency(F_CPU);
  // kernel.setTimeMultiplier(1.0); // some testing at 4x
  HAL_timer_init();
  std::thread simulation (simulation_loop);

  while(!initialised) std::this_thread::sleep_for(200ms);

  kernel.timerInit(2, 100000000);
  kernel.timerStart(2, 1000);
  kernel.timerEnable(2);

  while (!finished) {
    kernel.execute_loop();
  }
  kernel.kill();

  main_finished = true;
  simulation.join();
  write_serial.join();
  read_serial.join();

}


#endif // __PLAT_NATIVE_SIM__
