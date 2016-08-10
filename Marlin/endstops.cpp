/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
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

/**
 * endstops.cpp - A singleton object to manage endstops
 */

#include "Marlin.h"
#include "endstops.h"
#include "stepper.h"

// TEST_ENDSTOP: test the old and the current status of an endstop
#define TEST_ENDSTOP(ENDSTOP) (TEST(current_endstop_bits & old_endstop_bits, ENDSTOP))

Endstops endstops;

// public:

bool  Endstops::enabled = true,
      Endstops::enabled_globally = false;

volatile char Endstops::endstop_hit_bits; // use A_MIN, A_MAX, etc., as BIT value

byte Endstops::current_endstop_bits = 0,
     Endstops::old_endstop_bits = 0;

/**
 * Class and Instance Methods
 */

void Endstops::init() {

  #if HAS_A_MIN
    SET_INPUT(A_MIN_PIN);
    #if ENABLED(ENDSTOPPULLUP_AMIN)
      WRITE(A_MIN_PIN, HIGH);
    #endif
  #endif

  #if HAS_B_MIN
    SET_INPUT(B_MIN_PIN);
    #if ENABLED(ENDSTOPPULLUP_BMIN)
      WRITE(B_MIN_PIN, HIGH);
    #endif
  #endif

  #if HAS_C_MIN
    SET_INPUT(C_MIN_PIN);
    #if ENABLED(ENDSTOPPULLUP_CMIN)
      WRITE(C_MIN_PIN, HIGH);
    #endif
  #endif

  #if HAS_D_MIN
    SET_INPUT(D_MIN_PIN);
    #if ENABLED(ENDSTOPPULLUP_DMIN)
      WRITE(D_MIN_PIN, HIGH);
    #endif
  #endif

  #if HAS_A_MAX
    SET_INPUT(A_MAX_PIN);
    #if ENABLED(ENDSTOPPULLUP_AMAX)
      WRITE(A_MAX_PIN, HIGH);
    #endif
  #endif

  #if HAS_B_MAX
    SET_INPUT(B_MAX_PIN);
    #if ENABLED(ENDSTOPPULLUP_BMAX)
      WRITE(B_MAX_PIN, HIGH);
    #endif
  #endif

  #if HAS_C_MAX
    SET_INPUT(C_MAX_PIN);
    #if ENABLED(ENDSTOPPULLUP_CMAX)
      WRITE(C_MAX_PIN, HIGH);
    #endif
  #endif

  #if HAS_D_MAX
    SET_INPUT(D_MAX_PIN);
    #if ENABLED(ENDSTOPPULLUP_DMAX)
      WRITE(D_MAX_PIN, HIGH);
    #endif
  #endif

} // Endstops::init

void Endstops::report_state() {
  if (endstop_hit_bits) {
    #define _SET_STOP_CHAR(A,C) ;

    #define _ENDSTOP_HIT_ECHO(A,C) do{ \
      SERIAL_ECHOPAIR(" " STRINGIFY(A) ":", stepper.triggered_position_mm(A ##_AXIS)); \
      _SET_STOP_CHAR(A,C); }while(0)

    #define _ENDSTOP_HIT_TEST(A,C) \
      if (TEST(endstop_hit_bits, A ##_MIN) || TEST(endstop_hit_bits, A ##_MAX)) \
        _ENDSTOP_HIT_ECHO(A,C)

    SERIAL_ECHO_START;
    SERIAL_ECHOPGM(MSG_ENDSTOPS_HIT);
    _ENDSTOP_HIT_TEST(A, 'A');
    _ENDSTOP_HIT_TEST(B, 'B');
    _ENDSTOP_HIT_TEST(C, 'C');
    _ENDSTOP_HIT_TEST(D, 'D');
    SERIAL_EOL;

    hit_on_purpose();
  }
} // Endstops::report_state

void Endstops::M119() {
  SERIAL_PROTOCOLLNPGM(MSG_M119_REPORT);
  #if HAS_A_MIN
    SERIAL_PROTOCOLPGM(MSG_A_MIN);
    SERIAL_PROTOCOLLN(((READ(A_MIN_PIN)^A_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_A_MAX
    SERIAL_PROTOCOLPGM(MSG_A_MAX);
    SERIAL_PROTOCOLLN(((READ(A_MAX_PIN)^A_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_B_MIN
    SERIAL_PROTOCOLPGM(MSG_B_MIN);
    SERIAL_PROTOCOLLN(((READ(B_MIN_PIN)^B_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_B_MAX
    SERIAL_PROTOCOLPGM(MSG_B_MAX);
    SERIAL_PROTOCOLLN(((READ(B_MAX_PIN)^B_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_C_MIN
    SERIAL_PROTOCOLPGM(MSG_C_MIN);
    SERIAL_PROTOCOLLN(((READ(C_MIN_PIN)^C_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_C_MAX
    SERIAL_PROTOCOLPGM(MSG_C_MAX);
    SERIAL_PROTOCOLLN(((READ(C_MAX_PIN)^C_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_D_MIN
    SERIAL_PROTOCOLPGM(MSG_D_MIN);
    SERIAL_PROTOCOLLN(((READ(D_MIN_PIN)^D_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS_D_MAX
    SERIAL_PROTOCOLPGM(MSG_D_MAX);
    SERIAL_PROTOCOLLN(((READ(D_MAX_PIN)^D_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
} // Endstops::M119

// Check endstops - Called from ISR!
void Endstops::update() {

  #define _ENDSTOP_PIN(AXIS, MINMAX) AXIS ##_## MINMAX ##_PIN
  #define _ENDSTOP_INVERTING(AXIS, MINMAX) AXIS ##_## MINMAX ##_ENDSTOP_INVERTING
  #define _ENDSTOP_HIT(AXIS) SBI(endstop_hit_bits, _ENDSTOP(AXIS, MIN))
  #define _ENDSTOP(AXIS, MINMAX) AXIS ##_## MINMAX

  // UPDATE_ENDSTOP_BIT: set the current endstop bits for an endstop to its status
  #define UPDATE_ENDSTOP_BIT(AXIS, MINMAX) SET_BIT(current_endstop_bits, _ENDSTOP(AXIS, MINMAX), (READ(_ENDSTOP_PIN(AXIS, MINMAX)) != _ENDSTOP_INVERTING(AXIS, MINMAX)))
  // COPY_BIT: copy the value of COPY_BIT to BIT in bits
  #define COPY_BIT(bits, COPY_BIT, BIT) SET_BIT(bits, BIT, TEST(bits, COPY_BIT))

  #define UPDATE_ENDSTOP(AXIS,MINMAX) do { \
      UPDATE_ENDSTOP_BIT(AXIS, MINMAX); \
      if (TEST_ENDSTOP(_ENDSTOP(AXIS, MINMAX)) && stepper.current_block->steps[_AXIS(AXIS)] > 0) { \
        _ENDSTOP_HIT(AXIS); \
        stepper.endstop_triggered(_AXIS(AXIS)); \
      } \
    } while(0)

  if (stepper.motor_direction(A_AXIS)) { // stepping along -X axis (regular Cartesian bot)
    #if HAS_A_MIN
      UPDATE_ENDSTOP(A, MIN);
    #endif
  }
  else {
    #if HAS_A_MAX
      UPDATE_ENDSTOP(A, MAX);
    #endif
  }

  if (stepper.motor_direction(B_AXIS)) {
    #if HAS_B_MIN
      UPDATE_ENDSTOP(B, MIN);
    #endif
  }
  else {
    #if HAS_B_MAX
      UPDATE_ENDSTOP(B, MAX);
    #endif
  }

  if (stepper.motor_direction(C_AXIS)) {
    #if HAS_C_MIN
      UPDATE_ENDSTOP(C, MIN);
    #endif
  }
  else {
    #if HAS_C_MAX
      UPDATE_ENDSTOP(C, MAX);
    #endif
  }

  if (stepper.motor_direction(D_AXIS)) {
    #if HAS_D_MIN
      UPDATE_ENDSTOP(D, MIN);
    #endif
  }
  else {
    #if HAS_D_MAX
      UPDATE_ENDSTOP(D, MAX);
    #endif
  }

  old_endstop_bits = current_endstop_bits;

} // Endstops::update()
