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
#ifndef MARLIN_H
#define MARLIN_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "MarlinConfig.h"

#include "enum.h"
#include "types.h"
#include "fastio.h"

#ifdef USBCON
  #include "HardwareSerial.h"
  #define MYSERIAL Serial
#else
  #include "MarlinSerial.h"
  #define MYSERIAL customizedSerial
#endif

#include "WString.h"
#include "stopwatch.h"

#define SERIAL_CHAR(x) MYSERIAL.write(x)
#define SERIAL_EOL SERIAL_CHAR('\n')

#define SERIAL_PROTOCOLCHAR(x) SERIAL_CHAR(x)
#define SERIAL_PROTOCOL(x) MYSERIAL.print(x)
#define SERIAL_PROTOCOL_F(x,y) MYSERIAL.print(x,y)
#define SERIAL_PROTOCOLPGM(x) serialprintPGM(PSTR(x))
#define SERIAL_PROTOCOLLN(x) do{ MYSERIAL.print(x); SERIAL_EOL; }while(0)
#define SERIAL_PROTOCOLLNPGM(x) do{ serialprintPGM(PSTR(x "\n")); }while(0)

#define SERIAL_PROTOCOLPAIR(name, value) SERIAL_ECHOPAIR(name, value)
#define SERIAL_PROTOCOLLNPAIR(name, value) do{ SERIAL_ECHOPAIR(name, value); SERIAL_EOL; }while(0)

extern const char errormagic[] PROGMEM;
extern const char echomagic[] PROGMEM;

#define SERIAL_ERROR_START serialprintPGM(errormagic)
#define SERIAL_ERROR(x) SERIAL_PROTOCOL(x)
#define SERIAL_ERRORPGM(x) SERIAL_PROTOCOLPGM(x)
#define SERIAL_ERRORLN(x) SERIAL_PROTOCOLLN(x)
#define SERIAL_ERRORLNPGM(x) SERIAL_PROTOCOLLNPGM(x)

#define SERIAL_ECHO_START serialprintPGM(echomagic)
#define SERIAL_ECHO(x) SERIAL_PROTOCOL(x)
#define SERIAL_ECHOPGM(x) SERIAL_PROTOCOLPGM(x)
#define SERIAL_ECHOLN(x) SERIAL_PROTOCOLLN(x)
#define SERIAL_ECHOLNPGM(x) SERIAL_PROTOCOLLNPGM(x)

#define SERIAL_ECHOPAIR(name,value) (serial_echopair_P(PSTR(name),(value)))

void serial_echopair_P(const char* s_P, const char *v);
void serial_echopair_P(const char* s_P, char v);
void serial_echopair_P(const char* s_P, int v);
void serial_echopair_P(const char* s_P, long v);
void serial_echopair_P(const char* s_P, float v);
void serial_echopair_P(const char* s_P, double v);
void serial_echopair_P(const char* s_P, unsigned long v);
FORCE_INLINE void serial_echopair_P(const char* s_P, uint8_t v) { serial_echopair_P(s_P, (int)v); }
FORCE_INLINE void serial_echopair_P(const char* s_P, uint16_t v) { serial_echopair_P(s_P, (int)v); }
FORCE_INLINE void serial_echopair_P(const char* s_P, bool v) { serial_echopair_P(s_P, (int)v); }
FORCE_INLINE void serial_echopair_P(const char* s_P, void *v) { serial_echopair_P(s_P, (unsigned long)v); }

// Things to write to serial from Program memory. Saves 400 to 2k of RAM.
FORCE_INLINE void serialprintPGM(const char* str) {
  char ch;
  while ((ch = pgm_read_byte(str))) {
    MYSERIAL.write(ch);
    str++;
  }
}

void idle();

void manage_inactivity(bool ignore_stepper_queue = false);

#if HAS_A_ENABLE
  #define  enable_a() A_ENABLE_WRITE( A_ENABLE_ON)
  #define disable_a() do{ A_ENABLE_WRITE(!A_ENABLE_ON); axis_known_position[A_AXIS] = false; }while(0)
#else
  #define  enable_a() NOOP
  #define disable_a() NOOP
#endif

#if HAS_B_ENABLE
  #define  enable_b() B_ENABLE_WRITE( B_ENABLE_ON)
  #define disable_b() do{ B_ENABLE_WRITE(!B_ENABLE_ON); axis_known_position[B_AXIS] = false; }while(0)
#else
  #define  enable_b() NOOP
  #define disable_b() NOOP
#endif

#if HAS_C_ENABLE
  #define  enable_c() C_ENABLE_WRITE( C_ENABLE_ON)
  #define disable_c() do{ C_ENABLE_WRITE(!C_ENABLE_ON); axis_known_position[C_AXIS] = false; }while(0)
#else
  #define  enable_c() NOOP
  #define disable_c() NOOP
#endif

#if HAS_D_ENABLE
  #define  enable_d() D_ENABLE_WRITE( D_ENABLE_ON)
  #define disable_d() D_ENABLE_WRITE(!D_ENABLE_ON)
#else
  #define  enable_d() NOOP
  #define disable_d() NOOP
#endif

/**
 * The axis order in all axis related arrays is X, Y, Z, E
 */
#define _AXIS(AXIS) AXIS ##_AXIS

void enable_all_steppers();
void disable_all_steppers();

void FlushSerialRequestResend();
void ok_to_send();

void kill();

void quickstop_stepper();

extern uint8_t marlin_debug_flags;
#define DEBUGGING(F) (marlin_debug_flags & (DEBUG_## F))

extern bool Running;
inline bool IsRunning() { return  Running; }
inline bool IsStopped() { return !Running; }

bool enqueue_and_echo_command(const char* cmd, bool say_ok=false); //put a single ASCII command at the end of the current buffer or return false when it is full
void enqueue_and_echo_command_now(const char* cmd); // enqueue now, only return when the command has been enqueued
void enqueue_and_echo_commands_P(const char* cmd); //put one or many ASCII commands at the end of the current buffer, read from flash
void clear_command_queue();

extern millis_t previous_cmd_ms;
inline void refresh_cmd_timeout() { previous_cmd_ms = millis(); }

/**
 * Feedrate scaling and conversion
 */
extern int feedrate_percentage;

#define MMM_TO_MMS(MM_M) ((MM_M)/60.0)
#define MMS_TO_MMM(MM_S) ((MM_S)*60.0)
#define MMS_SCALED(MM_S) ((MM_S)*feedrate_percentage*0.01)

extern bool axis_relative_modes[];
extern bool axis_known_position[NUM_AXIS];
extern bool axis_homed[NUM_AXIS];
extern float current_position[NUM_AXIS];
extern float home_offset[NUM_AXIS];

#define LOGICAL_POSITION(POS, AXIS) (POS + home_offset[AXIS])
#define RAW_POSITION(POS, AXIS)     (POS - home_offset[AXIS])
#define LOGICAL_A_POSITION(POS)     LOGICAL_POSITION(POS, A_AXIS)
#define LOGICAL_B_POSITION(POS)     LOGICAL_POSITION(POS, B_AXIS)
#define LOGICAL_C_POSITION(POS)     LOGICAL_POSITION(POS, C_AXIS)
#define LOGICAL_D_POSITION(POS)     LOGICAL_POSITION(POS, D_AXIS)
#define RAW_A_POSITION(POS)         RAW_POSITION(POS, A_AXIS)
#define RAW_B_POSITION(POS)         RAW_POSITION(POS, B_AXIS)
#define RAW_C_POSITION(POS)         RAW_POSITION(POS, C_AXIS)
#define RAW_D_POSITION(POS)         RAW_POSITION(POS, D_AXIS)
#define RAW_CURRENT_POSITION(AXIS)  RAW_POSITION(current_position[AXIS], AXIS)

// GCode support for external objects
bool code_seen(char);
int code_value_int();
float code_value_temp_abs();
float code_value_temp_diff();

extern Stopwatch print_job_timer;

// Buzzer
#if HAS_BUZZER && PIN_EXISTS(BEEPER)
  #include "buzzer.h"
#endif

/**
 * Blocking movement and shorthand functions
 */
void do_blocking_move_to(const float &a, const float &b, const float &c, const float &d, float fr_mm_s=0.0);

#endif //MARLIN_H
