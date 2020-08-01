/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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
#pragma once

#include <cmath>

#include "Gpio.h"


class Heater: public Peripheral {
public:
  Heater(pin_type heater, pin_type adc);
  virtual ~Heater();
  void interrupt(GpioEvent& ev);
  void update();

  pin_type heater_pin, adc_pin;

  //heater element
  double heater_volts = 12.0;
  double heater_resistance = 3.6; // 40Watts

  //pwm sim
  uint64_t pwm_period = 0;
  uint64_t pwm_duty = 0;
  uint64_t pwm_hightick = 0;
  uint64_t pwm_lowtick = 0;
  uint64_t pwm_last_update = 0;

  //hotend block
  double hotend_ambient_temperature = 25.0;
  double hotend_temperature = 0.0;
  double hotend_energy = 0.0;
  double hotend_mass = 13.0; // g [approxiamte of 2x2x1 heatblock + nozzle]

  // // intermediates for sphere approximation
  // double hotend_density = 2.7; // gm/cm^3 (Aluminium)
  // double hotend_volume = hotend_mass / hotend_density; // cm^3
  // double hotend_radius = std::cbrt((hotend_volume * 3.0) / (4.0 * M_PI)); // cm
  // double hotend_surface_area = 4.0 * M_PI * hotend_radius * hotend_radius; // cm^2
  // // ****

  double hotend_surface_area = (16.0 + 4.0);// cm^2 [approxiamte of 2x2x1 heatblock + nozzle]
  double hotend_specific_heat = 0.897; // j/g/C (Aluminum)
  double hotend_convection_transfer = 0.001; // 0.001 W/cm^2 . C is an approximate often used for convective heat transfer into slow moving air

  //adc
  double adc_pullup_resistance = 4700;
  uint32_t adc_resolution = 12;
};
