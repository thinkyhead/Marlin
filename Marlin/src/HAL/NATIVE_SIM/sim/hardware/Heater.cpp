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
#ifdef __PLAT_NATIVE_SIM__

#include <Arduino.h>
#include "Heater.h"

constexpr double absolute_zero_offset = -273.15;
double thermistor_ext_coef[] = {
  7.611428226793945e-04,
  2.011100481838449e-04,
  1.914201231699539e-06,
  1.561937567632929e-08
};

double temperature_to_resistance(double t) {
	double r, u, v, p, q, b, c, d;
	t = t - absolute_zero_offset;
	d = (thermistor_ext_coef[0] - 1.0 / t) / thermistor_ext_coef[3];
	c = thermistor_ext_coef[1] / thermistor_ext_coef[3];
	b = thermistor_ext_coef[2] / thermistor_ext_coef[3];
	q = 2.0 / 27.0 * b * b * b - 1.0 / 3.0 * b * c + d;
	p = c - 1.0 / 3.0 * b * b;
	v = - pow(q / 2.0 + sqrt(q * q / 4.0 + p * p * p / 27.0), 1.0 / 3.0);
	u =   pow(-q / 2.0 + sqrt(q * q / 4.0 + p * p * p / 27.0), 1.0 / 3.0);
	r  = exp(u + v - b / 3.0);
	return r;
}

Heater::Heater(pin_t heater, pin_t adc) {
  heater_pin = heater;
  adc_pin = analogInputToDigitalPin(adc);

  Gpio::attach(adc_pin, std::bind(&Heater::interrupt, this, std::placeholders::_1));
  Gpio::attach(heater_pin, std::bind(&Heater::interrupt, this, std::placeholders::_1));
  hotend_energy = hotend_ambient_temperature * (hotend_specific_heat * hotend_mass);
  hotend_temperature = hotend_ambient_temperature;
}

Heater::~Heater() {
}

void Heater::update() {

}

// models energy transfer but not time lag as it tranfers through the medium.
void Heater::interrupt(GpioEvent& ev) {
  if (ev.event == ev.RISE && ev.pin_id == heater_pin) {
    if (pwm_hightick) pwm_period = ev.timestamp - pwm_hightick;
    pwm_hightick = ev.timestamp;

  } else if ((ev.event == ev.NOP || ev.event == ev.FALL) && ev.pin_id == heater_pin) {
    double time_delta = kernel.ticksToNanos(ev.timestamp - pwm_last_update) / (double)kernel.ONE_BILLION;
    double energy_in = pwm_lowtick < pwm_hightick ? ((heater_volts * heater_volts) / heater_resistance) * time_delta : 0;
    double energy_out = ((hotend_convection_transfer * hotend_surface_area * ( hotend_energy / (hotend_specific_heat * hotend_mass) - hotend_ambient_temperature)) * time_delta);
    hotend_energy += energy_in - energy_out;
    pwm_last_update = ev.timestamp;

    hotend_temperature = hotend_energy / (hotend_specific_heat * hotend_mass);

    if (ev.event == ev.FALL) {
      pwm_lowtick = ev.timestamp;
      pwm_duty = ev.timestamp - pwm_hightick;
    }

  } else if (ev.event == ev.GET_VALUE && ev.pin_id == adc_pin) {
    double thermistor_resistance = temperature_to_resistance(hotend_temperature);
    uint32_t adc_reading = (uint32_t)((((1U << adc_resolution) -1)  * thermistor_resistance) / (adc_pullup_resistance + thermistor_resistance));
    Gpio::pin_map[adc_pin].value = adc_reading;
  }
}

#endif // __PLAT_NATIVE_SIM__
