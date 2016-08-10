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
 *
 * About Marlin
 *
 * This firmware is a mashup between Sprinter and grbl.
 *  - https://github.com/kliment/Sprinter
 *  - https://github.com/simen/grbl/tree
 *
 * It has preliminary support for Matthew Roberts advance algorithm
 *  - http://reprap.org/pipermail/reprap-dev/2011-May/003323.html
 */

#include "Marlin.h"
#include "planner.h"
#include "stepper.h"
#include "endstops.h"
#include "configuration_store.h"
#include "language.h"
#include "pins_arduino.h"
#include "math.h"
#include "types.h"

#if HAS_DIGIPOTSS
  #include <SPI.h>
#endif

#if ENABLED(DAC_STEPPER_CURRENT)
  #include "stepper_dac.h"
#endif

#if ENABLED(EXPERIMENTAL_I2CBUS)
  #include "twibus.h"
#endif

/**
 * Look here for descriptions of G-codes:
 *  - http://linuxcnc.org/handbook/gcode/g-code.html
 *  - http://objects.reprap.org/wiki/Mendel_User_Manual:_RepRapGCodes
 *
 * Help us document these G-codes online:
 *  - https://github.com/MarlinFirmware/Marlin/wiki/G-Code-in-Marlin
 *  - http://reprap.org/wiki/G-code
 *
 * -----------------
 * Implemented Codes
 * -----------------
 *
 * "G" Codes
 *
 * G0  -> G1
 * G1  - Coordinated Movement X Y Z E
 * G4  - Dwell S<seconds> or P<milliseconds>
 * G28 - Home one or more axes
 * G90 - Use Absolute Coordinates
 * G91 - Use Relative Coordinates
 * G92 - Set current position to coordinates given
 *
 * "M" Codes
 *
 * M0   - Unconditional stop - Wait for user to press a button on the LCD (Only if ULTRA_LCD is enabled)
 * M1   - Same as M0
 * M17  - Enable/Power all stepper motors
 * M18  - Disable all stepper motors; same as M84
 * M42  - Change pin status via gcode Use M42 Px Sy to set pin x to value y, when omitting Px the onboard led will be used.
 * M80  - Turn on Power Supply
 * M81  - Turn off Power Supply
 * M84  - Disable steppers until next move,
 *        or use S<seconds> to specify an inactivity timeout, after which the steppers will be disabled.  S0 to disable the timeout.
 * M85  - Set inactivity shutdown timer with parameter S<seconds>. To disable set zero (default)
 * M92  - Set planner.axis_steps_per_mm - same syntax as G92
 * M104 - Set extruder target temp
 * M110 - Set the current line number
 * M111 - Set debug flags with S<mask>. See flag bits defined in enum.h.
 * M112 - Emergency stop
 * M114 - Output current position to serial port
 * M115 - Capabilities string
 * M119 - Output Endstop status to serial port
 * M120 - Enable endstop detection
 * M121 - Disable endstop detection
 * M201 - Set max acceleration in units/s^2 for print moves (M201 X1000 Y1000)
 * M203 - Set maximum feedrate that your machine can sustain (M203 X200 Y200 Z300 E10000) in units/sec
 * M204 - Set default acceleration: P for Printing moves, R for Retract only (no X, Y, Z) moves and T for Travel (non printing) moves (ex. M204 P800 T3000 R9000) in units/sec^2
 * M205 - Set advanced settings. Current units apply:
            S<print> T<travel> minimum speeds
            B<minimum segment time>
            Z<max Z jerk>
 * M206 - Set additional homing offset
 * M220 - Set Feedrate Percentage: S<percent> ("FR" on your LCD)
 * M226 - Wait until the specified pin reaches the state required: P<pin number> S<pin state>
 * M300 - Play beep sound S<frequency Hz> P<duration ms>
 * M400 - Finish all moves
 * M410 - Quickstop. Abort all the planned moves
 * M500 - Store parameters in EEPROM
 * M501 - Read parameters from EEPROM (if you need reset them after you changed them temporarily).
 * M502 - Revert to the default "factory settings". You still need to store them in EEPROM afterwards if you want to.
 * M503 - Print the current settings (from memory not from EEPROM). Use S0 to leave off headings.
 * M907 - Set digital trimpot motor current using axis codes.
 * M908 - Control digital trimpot directly.
 * M909 - DAC_STEPPER_CURRENT: Print digipot/DAC current value
 * M910 - DAC_STEPPER_CURRENT: Commit digipot/DAC value to external EEPROM via I2C
 * M350 - Set microstepping mode.
 * M351 - Toggle MS1 MS2 pins directly.
 *
 * ************ Custom codes - This can change to suit future G-code regulations
 * M100 - Watch Free Memory (For Debugging Only)
 * M999 - Restart after being stopped by error
 *
 */

#if ENABLED(M100_FREE_MEMORY_WATCHER)
  void gcode_M100();
#endif

#if ENABLED(EXPERIMENTAL_I2CBUS)
  TWIBus i2c;
#endif

bool Running = true;

uint8_t marlin_debug_flags = DEBUG_NONE;

float current_position[NUM_AXIS] = { 0.0 };
static float destination[NUM_AXIS] = { 0.0 };
bool axis_known_position[NUM_AXIS] = { false };
bool axis_homed[NUM_AXIS] = { false };

static long gcode_N, gcode_LastN, Stopped_gcode_LastN = 0;

static char command_queue[BUFSIZE][MAX_CMD_SIZE];
static char* current_command, *current_command_args;
static uint8_t cmd_queue_index_r = 0,
               cmd_queue_index_w = 0,
               commands_in_queue = 0;

/**
 * Feed rates are often configured with mm/m
 * but the planner and stepper like mm/s units.
 */
const float homing_feedrate_mm_s = MMM_TO_MMS(HOMING_FEEDRATE_Z);
static float feedrate_mm_s = MMM_TO_MMS(1500.0), saved_feedrate_mm_s;
int feedrate_percentage = 100, saved_feedrate_percentage;

bool axis_relative_modes[] = AXIS_RELATIVE_MODES;

// This offset is added to the configured home position.
// Set by M206, M428, or menu item. Saved to EEPROM.
float home_offset[NUM_AXIS] = { 0 };

// Relative Mode. Enable with G91, disable with G90.
static bool relative_mode = false;

const char errormagic[] PROGMEM = "Error:";
const char echomagic[] PROGMEM = "echo:";
const char axis_codes[NUM_AXIS] = {'A', 'B', 'C', 'D'};

static int serial_count = 0;

// GCode parameter pointer used by code_seen(), code_value_float(), etc.
static char* seen_pointer;

// Next Immediate GCode Command pointer. NULL if none.
const char* queued_commands_P = NULL;

const int sensitive_pins[] = SENSITIVE_PINS; ///< Sensitive pin list for M42

// Inactivity shutdown
millis_t previous_cmd_ms = 0;
static millis_t max_inactive_time = 0;
static millis_t stepper_inactive_time = (DEFAULT_STEPPER_DEACTIVE_TIME) * 1000UL;

// Buzzer - I2C on the LCD or a BEEPER_PIN
#if ENABLED(LCD_USE_I2C_BUZZER)
  #define BUZZ(d,f) lcd_buzz(d, f)
#elif HAS_BUZZER
  Buzzer buzzer;
  #define BUZZ(d,f) buzzer.tone(d, f)
#else
  #define BUZZ(d,f) NOOP
#endif

static bool send_ok[BUFSIZE];

/**
 * ***************************************************************************
 * ******************************** FUNCTIONS ********************************
 * ***************************************************************************
 */

void stop();

void get_available_commands();
void process_next_command();
void prepare_move_to_destination();
void set_current_from_steppers_for_axis(AxisEnum axis);

void serial_echopair_P(const char* s_P, const char *v)   { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char* s_P, char v)          { serialprintPGM(s_P); SERIAL_CHAR(v); }
void serial_echopair_P(const char* s_P, int v)           { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char* s_P, long v)          { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char* s_P, float v)         { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char* s_P, double v)        { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char* s_P, unsigned long v) { serialprintPGM(s_P); SERIAL_ECHO(v); }

static void report_current_position();

#if ENABLED(DEBUG_LEVELING_FEATURE)
  void print_abcd(const char* prefix, const char* suffix, const float &a, const float &b, const float &c, const float &d) {
    serialprintPGM(prefix);
    SERIAL_ECHOPAIR("(", a);
    SERIAL_ECHOPAIR(", ", b);
    SERIAL_ECHOPAIR(", ", c);
    SERIAL_ECHOPAIR(", ", d);
    SERIAL_ECHOPGM(")");

    if (suffix) serialprintPGM(suffix);
    else SERIAL_EOL;
  }

  void print_abcd(const char* prefix, const char* suffix, const float abcd[]) {
    print_abcd(prefix, suffix, abcd[A_AXIS], abcd[B_AXIS], abcd[C_AXIS], abcd[D_AXIS]);
  }

  #define DEBUG_POS(SUFFIX,VAR) do { \
    print_abcd(PSTR(STRINGIFY(VAR) "="), PSTR(" : " SUFFIX "\n"), VAR); } while(0)
#endif

/**
 * sync_plan_position
 * Set planner / stepper positions to the cartesian current_position.
 * The stepper code translates these coordinates into step units.
 * Allows translation between steps and millimeters for cartesian & core robots
 */
inline void sync_plan_position() {
  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) DEBUG_POS("sync_plan_position", current_position);
  #endif
  planner.set_position_mm(current_position[A_AXIS], current_position[B_AXIS], current_position[C_AXIS], current_position[D_AXIS]);
}

extern "C" {
  extern unsigned int __bss_end;
  extern unsigned int __heap_start;
  extern void* __brkval;

  int freeMemory() {
    int free_memory;
    if ((int)__brkval == 0)
      free_memory = ((int)&free_memory) - ((int)&__bss_end);
    else
      free_memory = ((int)&free_memory) - ((int)__brkval);
    return free_memory;
  }
}

#if ENABLED(DIGIPOT_I2C)
  extern void digipot_i2c_set_current(int channel, float current);
  extern void digipot_i2c_init();
#endif

/**
 * Inject the next "immediate" command, when possible.
 * Return true if any immediate commands remain to inject.
 */
static bool drain_queued_commands_P() {
  if (queued_commands_P != NULL) {
    size_t i = 0;
    char c, cmd[30];
    strncpy_P(cmd, queued_commands_P, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';
    while ((c = cmd[i]) && c != '\n') i++; // find the end of this gcode command
    cmd[i] = '\0';
    if (enqueue_and_echo_command(cmd)) {   // success?
      if (c)                               // newline char?
        queued_commands_P += i + 1;        // advance to the next command
      else
        queued_commands_P = NULL;          // nul char? no more commands
    }
  }
  return (queued_commands_P != NULL);      // return whether any more remain
}

/**
 * Record one or many commands to run from program memory.
 * Aborts the current queue, if any.
 * Note: drain_queued_commands_P() must be called repeatedly to drain the commands afterwards
 */
void enqueue_and_echo_commands_P(const char* pgcode) {
  queued_commands_P = pgcode;
  drain_queued_commands_P(); // first command executed asap (when possible)
}

void clear_command_queue() {
  cmd_queue_index_r = cmd_queue_index_w;
  commands_in_queue = 0;
}

/**
 * Once a new command is in the ring buffer, call this to commit it
 */
inline void _commit_command(bool say_ok) {
  send_ok[cmd_queue_index_w] = say_ok;
  cmd_queue_index_w = (cmd_queue_index_w + 1) % BUFSIZE;
  commands_in_queue++;
}

/**
 * Copy a command directly into the main command buffer, from RAM.
 * Returns true if successfully adds the command
 */
inline bool _enqueuecommand(const char* cmd, bool say_ok=false) {
  if (*cmd == ';' || commands_in_queue >= BUFSIZE) return false;
  strcpy(command_queue[cmd_queue_index_w], cmd);
  _commit_command(say_ok);
  return true;
}

void enqueue_and_echo_command_now(const char* cmd) {
  while (!enqueue_and_echo_command(cmd)) idle();
}

/**
 * Enqueue with Serial Echo
 */
bool enqueue_and_echo_command(const char* cmd, bool say_ok/*=false*/) {
  if (_enqueuecommand(cmd, say_ok)) {
    SERIAL_ECHO_START;
    SERIAL_ECHOPGM(MSG_Enqueueing);
    SERIAL_ECHO(cmd);
    SERIAL_ECHOLNPGM("\"");
    return true;
  }
  return false;
}

void setup_killpin() {
  #if HAS_KILL
    SET_INPUT(KILL_PIN);
    WRITE(KILL_PIN, HIGH);
  #endif
}

// Set home pin
void setup_homepin(void) {
  #if HAS_HOME
    SET_INPUT(HOME_PIN);
    WRITE(HOME_PIN, HIGH);
  #endif
}

void setup_powerhold() {
  #if HAS_SUICIDE
    OUT_WRITE(SUICIDE_PIN, HIGH);
  #endif
  #if HAS_POWER_SWITCH
    #if ENABLED(PS_DEFAULT_OFF)
      OUT_WRITE(PS_ON_PIN, PS_ON_ASLEEP);
    #else
      OUT_WRITE(PS_ON_PIN, PS_ON_AWAKE);
    #endif
  #endif
}

void suicide() {
  #if HAS_SUICIDE
    OUT_WRITE(SUICIDE_PIN, LOW);
  #endif
}

#if ENABLED(EXPERIMENTAL_I2CBUS) && I2C_SLAVE_ADDRESS > 0

  //
  // [A3D] Handle commands to the Z Axis controller here!
  //
  void i2c_on_receive(int bytes) { // just echo all bytes received to serial
    static char buff[33];
    buff[i2c.capture(buff, 32)] = '\0';
  }

  void i2c_on_request() {          // just send dummy data for now
    i2c.reply("Hello World!\n");
  }

#endif

/**
 * Marlin entry-point: Set up before the program loop
 *  - Set up the kill pin, filament runout, power hold
 *  - Start the serial port
 *  - Print startup messages and diagnostics
 *  - Get EEPROM or default settings
 *  - Initialize managers for:
 *    • temperature
 *    • planner
 *    • watchdog
 *    • stepper
 *    • photo pin
 *    • servos
 *    • LCD controller
 *    • Digipot I2C
 *    • Z probe sled
 *    • status LEDs
 */
void setup() {

  #ifdef DISABLE_JTAG
    // Disable JTAG on AT90USB chips to free up pins for IO
    MCUCR = 0x80;
    MCUCR = 0x80;
  #endif

  setup_killpin();

  setup_powerhold();

  MYSERIAL.begin(BAUDRATE);
  SERIAL_PROTOCOLLNPGM("start");
  SERIAL_ECHO_START;

  // Check startup - does nothing if bootloader sets MCUSR to 0
  byte mcu = MCUSR;
  if (mcu & 1) SERIAL_ECHOLNPGM(MSG_POWERUP);
  if (mcu & 2) SERIAL_ECHOLNPGM(MSG_EXTERNAL_RESET);
  if (mcu & 4) SERIAL_ECHOLNPGM(MSG_BROWNOUT_RESET);
  if (mcu & 8) SERIAL_ECHOLNPGM(MSG_WATCHDOG_RESET);
  if (mcu & 32) SERIAL_ECHOLNPGM(MSG_SOFTWARE_RESET);
  MCUSR = 0;

  SERIAL_ECHOPGM(MSG_MARLIN);
  SERIAL_ECHOLNPGM(" " SHORT_BUILD_VERSION);

  #ifdef STRING_DISTRIBUTION_DATE
    #ifdef STRING_CONFIG_H_AUTHOR
      SERIAL_ECHO_START;
      SERIAL_ECHOPGM(MSG_CONFIGURATION_VER);
      SERIAL_ECHOPGM(STRING_DISTRIBUTION_DATE);
      SERIAL_ECHOPGM(MSG_AUTHOR);
      SERIAL_ECHOLNPGM(STRING_CONFIG_H_AUTHOR);
      SERIAL_ECHOPGM("Compiled: ");
      SERIAL_ECHOLNPGM(__DATE__);
    #endif // STRING_CONFIG_H_AUTHOR
  #endif // STRING_DISTRIBUTION_DATE

  SERIAL_ECHO_START;
  SERIAL_ECHOPGM(MSG_FREE_MEMORY);
  SERIAL_ECHO(freeMemory());
  SERIAL_ECHOPGM(MSG_PLANNER_BUFFER_BYTES);
  SERIAL_ECHOLN((int)sizeof(block_t)*BLOCK_BUFFER_SIZE);

  // Send "ok" after commands by default
  for (int8_t i = 0; i < BUFSIZE; i++) send_ok[i] = true;

  // Load data from EEPROM if available (or use defaults)
  // This also updates variables in the planner, elsewhere
  Config_RetrieveSettings();

  // Initialize current position based on home_offset
  memcpy(current_position, home_offset, sizeof(home_offset));

  // Vital to init stepper/planner equivalent for current_position
  sync_plan_position();

  stepper.init();    // Initialize stepper, this enables interrupts!

  #if HAS_CONTROLLERFAN
    SET_OUTPUT(CONTROLLERFAN_PIN); //Set pin used for driver cooling fan
  #endif

  #if ENABLED(DIGIPOT_I2C)
    digipot_i2c_init();
  #endif

  #if ENABLED(DAC_STEPPER_CURRENT)
    dac_init();
  #endif

  setup_homepin();

  #if ENABLED(EXPERIMENTAL_I2CBUS) && I2C_SLAVE_ADDRESS > 0
    i2c.onReceive(i2c_on_receive);
    i2c.onRequest(i2c_on_request);
  #endif
}

/**
 * The main Marlin program loop
 *
 *  - Save or log commands to SD
 *  - Process available commands (if not saving)
 *  - Call heater manager
 *  - Call inactivity manager
 *  - Call endstop manager
 *  - Call LCD update
 */
void loop() {
  if (commands_in_queue < BUFSIZE) get_available_commands();

  if (commands_in_queue) {

    process_next_command();

    // The queue may be reset by a command handler or by code invoked by idle() within a handler
    if (commands_in_queue) {
      --commands_in_queue;
      cmd_queue_index_r = (cmd_queue_index_r + 1) % BUFSIZE;
    }
  }
  endstops.report_state();
  idle();
}

void gcode_line_error(const char* err, bool doFlush = true) {
  SERIAL_ERROR_START;
  serialprintPGM(err);
  SERIAL_ERRORLN(gcode_LastN);
  //Serial.println(gcode_N);
  if (doFlush) FlushSerialRequestResend();
  serial_count = 0;
}

inline void get_serial_commands() {
  static char serial_line_buffer[MAX_CMD_SIZE];
  static boolean serial_comment_mode = false;

  // If the command buffer is empty for too long,
  // send "wait" to indicate Marlin is still waiting.
  #if defined(NO_TIMEOUTS) && NO_TIMEOUTS > 0
    static millis_t last_command_time = 0;
    millis_t ms = millis();
    if (commands_in_queue == 0 && !MYSERIAL.available() && ELAPSED(ms, last_command_time + NO_TIMEOUTS)) {
      SERIAL_ECHOLNPGM(MSG_WAIT);
      last_command_time = ms;
    }
  #endif

  /**
   * Loop while serial characters are incoming and the queue is not full
   */
  while (commands_in_queue < BUFSIZE && MYSERIAL.available() > 0) {

    char serial_char = MYSERIAL.read();

    /**
     * If the character ends the line
     */
    if (serial_char == '\n' || serial_char == '\r') {

      serial_comment_mode = false; // end of line == end of comment

      if (!serial_count) continue; // skip empty lines

      serial_line_buffer[serial_count] = 0; // terminate string
      serial_count = 0; //reset buffer

      char* command = serial_line_buffer;

      while (*command == ' ') command++; // skip any leading spaces
      char* npos = (*command == 'N') ? command : NULL; // Require the N parameter to start the line
      char* apos = strchr(command, '*');

      if (npos) {

        boolean M110 = strstr_P(command, PSTR("M110")) != NULL;

        if (M110) {
          char* n2pos = strchr(command + 4, 'N');
          if (n2pos) npos = n2pos;
        }

        gcode_N = strtol(npos + 1, NULL, 10);

        if (gcode_N != gcode_LastN + 1 && !M110) {
          gcode_line_error(PSTR(MSG_ERR_LINE_NO));
          return;
        }

        if (apos) {
          byte checksum = 0, count = 0;
          while (command[count] != '*') checksum ^= command[count++];

          if (strtol(apos + 1, NULL, 10) != checksum) {
            gcode_line_error(PSTR(MSG_ERR_CHECKSUM_MISMATCH));
            return;
          }
          // if no errors, continue parsing
        }
        else {
          gcode_line_error(PSTR(MSG_ERR_NO_CHECKSUM));
          return;
        }

        gcode_LastN = gcode_N;
        // if no errors, continue parsing
      }
      else if (apos) { // No '*' without 'N'
        gcode_line_error(PSTR(MSG_ERR_NO_LINENUMBER_WITH_CHECKSUM), false);
        return;
      }

      // Movement commands alert when stopped
      if (IsStopped()) {
        char* gpos = strchr(command, 'G');
        if (gpos) {
          int codenum = strtol(gpos + 1, NULL, 10);
          switch (codenum) {
            case 0:
            case 1:
            case 2:
            case 3:
              SERIAL_ERRORLNPGM(MSG_ERR_STOPPED);
              break;
          }
        }
      }

      #if DISABLED(EMERGENCY_PARSER)
        // If command was e-stop process now
        if (strcmp(command, "M112") == 0) kill();
        if (strcmp(command, "M410") == 0) { quickstop_stepper(); }
      #endif

      #if defined(NO_TIMEOUTS) && NO_TIMEOUTS > 0
        last_command_time = ms;
      #endif

      // Add the command to the queue
      _enqueuecommand(serial_line_buffer, true);
    }
    else if (serial_count >= MAX_CMD_SIZE - 1) {
      // Keep fetching, but ignore normal characters beyond the max length
      // The command will be injected when EOL is reached
    }
    else if (serial_char == '\\') {  // Handle escapes
      if (MYSERIAL.available() > 0) {
        // if we have one more character, copy it over
        serial_char = MYSERIAL.read();
        if (!serial_comment_mode) serial_line_buffer[serial_count++] = serial_char;
      }
      // otherwise do nothing
    }
    else { // it's not a newline, carriage return or escape char
      if (serial_char == ';') serial_comment_mode = true;
      if (!serial_comment_mode) serial_line_buffer[serial_count++] = serial_char;
    }

  } // queue has space, serial has data
}

/**
 * Add to the circular command queue the next command from:
 *  - The command-injection queue (queued_commands_P)
 *  - The active serial input (usually USB)
 *  - The SD card file being actively printed
 */
void get_available_commands() {
  // if any immediate commands remain, don't get other commands yet
  if (drain_queued_commands_P()) return;
  get_serial_commands();
}

inline bool code_has_value() {
  int i = 1;
  char c = seen_pointer[i];
  while (c == ' ') c = seen_pointer[++i];
  if (c == '-' || c == '+') c = seen_pointer[++i];
  if (c == '.') c = seen_pointer[++i];
  return NUMERIC(c);
}

inline float code_value_float() {
  float ret;
  char* e = strchr(seen_pointer, 'E');
  if (e) {
    *e = 0;
    ret = strtod(seen_pointer + 1, NULL);
    *e = 'E';
  }
  else
    ret = strtod(seen_pointer + 1, NULL);
  return ret;
}

inline unsigned long code_value_ulong() { return strtoul(seen_pointer + 1, NULL, 10); }

inline long code_value_long() { return strtol(seen_pointer + 1, NULL, 10); }

inline int code_value_int() { return (int)strtol(seen_pointer + 1, NULL, 10); }

inline uint16_t code_value_ushort() { return (uint16_t)strtoul(seen_pointer + 1, NULL, 10); }

inline uint8_t code_value_byte() { return (uint8_t)(constrain(strtol(seen_pointer + 1, NULL, 10), 0, 255)); }

inline bool code_value_bool() { return code_value_byte() > 0; }

inline float code_value_linear_units() { return code_value_float(); }
inline float code_value_axis_units() { return code_value_float(); }
inline float code_value_per_axis_unit() { return code_value_float(); }

float code_value_temp_abs() { return code_value_float(); }
float code_value_temp_diff() { return code_value_float(); }

FORCE_INLINE millis_t code_value_millis() { return code_value_ulong(); }
inline millis_t code_value_millis_from_seconds() { return code_value_float() * 1000; }

bool code_seen(char code) {
  seen_pointer = strchr(current_command_args, code);
  return (seen_pointer != NULL); // Return TRUE if the code-letter was found
}

static const signed char home_dir = Z_HOME_DIR;

/**
 * Change the home offset for an axis, update the current
 * position and the software endstops to retain the same
 * relative distance to the new home.
 *
 * Since this changes the current_position, code should
 * call sync_plan_position soon after this.
 */
static void set_home_offset(AxisEnum axis, float v) {
  current_position[axis] += v - home_offset[axis];
  home_offset[axis] = v;
}

static void set_axis_is_at_home(AxisEnum axis) {
  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) {
      SERIAL_ECHOPAIR(">>> set_axis_is_at_home(", axis_codes[axis]);
      SERIAL_ECHOLNPGM(")");
    }
  #endif

  current_position[axis] = LOGICAL_POSITION(Z_HOME_POS, axis);

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) {
      SERIAL_ECHOPAIR("> home_offset[", axis_codes[axis]);
      SERIAL_ECHOPAIR("] = ", home_offset[axis]);
      SERIAL_EOL;
      DEBUG_POS("", current_position);
      SERIAL_ECHOPAIR("<<< set_axis_is_at_home(", axis_codes[axis]);
      SERIAL_ECHOLNPGM(")");
    }
  #endif
}

/**
 * Some planner shorthand inline functions
 */
inline float get_homing_bump_feedrate(AxisEnum axis) {
  int hbd = HOMING_BUMP_DIVISOR;
  if (hbd < 1) {
    hbd = 10;
    SERIAL_ECHO_START;
    SERIAL_ECHOLNPGM("Warning: Homing Bump Divisor < 1");
  }
  return homing_feedrate_mm_s / hbd;
}
//
// line_to_current_position
// Move the planner to the current position from wherever it last moved
// (or from wherever it has been told it is located).
//
inline void line_to_current_position() {
  planner.buffer_line(current_position[A_AXIS], current_position[B_AXIS], current_position[C_AXIS], current_position[D_AXIS], feedrate_mm_s);
}

inline void line_to_z(float zPosition) {
  planner.buffer_line(current_position[A_AXIS], current_position[B_AXIS], zPosition, current_position[D_AXIS], feedrate_mm_s);
}

inline void line_to_axis_pos(AxisEnum axis, float where, float fr_mm_s = 0.0) {
  float old_feedrate_mm_s = feedrate_mm_s;
  current_position[axis] = where;
  feedrate_mm_s = (fr_mm_s != 0.0) ? fr_mm_s : homing_feedrate_mm_s;
  planner.buffer_line(current_position[A_AXIS], current_position[B_AXIS], current_position[C_AXIS], current_position[D_AXIS], feedrate_mm_s);
  stepper.synchronize();
  feedrate_mm_s = old_feedrate_mm_s;
}

//
// line_to_destination
// Move the planner, not necessarily synced with current_position
//
inline void line_to_destination(float fr_mm_s) {
  planner.buffer_line(destination[A_AXIS], destination[B_AXIS], destination[C_AXIS], destination[D_AXIS], fr_mm_s);
}
inline void line_to_destination() { line_to_destination(feedrate_mm_s); }

inline void set_current_to_destination() { memcpy(current_position, destination, sizeof(current_position)); }
inline void set_destination_to_current() { memcpy(destination, current_position, sizeof(destination)); }

/**
 *  Plan a move to (X, Y, Z) and set the current_position
 *  The final current_position may not be the one that was requested
 */
inline void do_blocking_move_to(const float &a, const float &b, const float &c, const float &d, float fr_mm_s/*=0.0*/) {

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) print_abcd(PSTR(">>> do_blocking_move_to"), NULL, a, b, c, d);
  #endif

  float old_feedrate_mm_s = feedrate_mm_s;
  feedrate_mm_s = (fr_mm_s != 0.0) ? fr_mm_s : homing_feedrate_mm_s;
  current_position[A_AXIS] = a;
  current_position[B_AXIS] = b;
  current_position[C_AXIS] = c;
  current_position[D_AXIS] = d;
  line_to_current_position();
  stepper.synchronize();
  feedrate_mm_s = old_feedrate_mm_s;
}

//
// Prepare to do endstop or probe moves
// with custom feedrates.
//
//  - Save current feedrates
//  - Reset the rate multiplier
//  - Reset the command timeout
//  - Enable the endstops (for endstop moves)
//
static void setup_for_endstop_move() {
  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) DEBUG_POS("setup_for_endstop_move", current_position);
  #endif
  saved_feedrate_mm_s = feedrate_mm_s;
  saved_feedrate_percentage = feedrate_percentage;
  feedrate_percentage = 100;
  refresh_cmd_timeout();
}

static void clean_up_after_endstop_move() {
  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) DEBUG_POS("clean_up_after_endstop_move", current_position);
  #endif
  feedrate_mm_s = saved_feedrate_mm_s;
  feedrate_percentage = saved_feedrate_percentage;
  refresh_cmd_timeout();
}

/**
 * Home an individual axis
 */

#define HOMEAXIS(LETTER) homeaxis(LETTER##_AXIS)

static void homeaxis(AxisEnum axis) {
  #define HOMEAXIS_DO(LETTER) \
    ((LETTER##_MIN_PIN > -1 && Z_HOME_DIR == -1) || (LETTER##_MAX_PIN > -1 && Z_HOME_DIR == 1))

  if (!(axis == A_AXIS ? HOMEAXIS_DO(A) :
        axis == B_AXIS ? HOMEAXIS_DO(B) :
        axis == C_AXIS ? HOMEAXIS_DO(C) :
        axis == D_AXIS ? HOMEAXIS_DO(D) : false)) return;

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) {
      SERIAL_ECHOPAIR(">>> homeaxis(", axis_codes[axis]);
      SERIAL_ECHOLNPGM(")");
    }
  #endif

  // Set the axis position as setup for the move
  current_position[axis] = 0;
  sync_plan_position();

  // Move towards the endstop until an endstop is triggered
  line_to_axis_pos(axis, 1.5 * (Z_MAX_LENGTH) * (Z_HOME_DIR));

  // Set the axis position as setup for the move
  current_position[axis] = 0;
  sync_plan_position();

  // Move away from the endstop by the axis HOME_BUMP_MM
  line_to_axis_pos(axis, -(Z_HOME_BUMP_MM) * (Z_HOME_DIR));

  // Move slowly towards the endstop until triggered
  line_to_axis_pos(axis, 2 * (Z_HOME_BUMP_MM) * (Z_HOME_DIR), get_homing_bump_feedrate(axis));

  // reset current_position to 0 to reflect hitting endpoint
  current_position[axis] = 0;
  sync_plan_position();

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) DEBUG_POS("> TRIGGER ENDSTOP", current_position);
  #endif

  // Set the axis position to its home position (plus home offsets)
  set_axis_is_at_home(axis);

  sync_plan_position();

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) DEBUG_POS("> AFTER set_axis_is_at_home", current_position);
  #endif

  destination[axis] = current_position[axis];
  endstops.hit_on_purpose(); // clear endstop hit flags
  axis_known_position[axis] = true;
  axis_homed[axis] = true;

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) {
      SERIAL_ECHOPAIR("<<< homeaxis(", axis_codes[axis]);
      SERIAL_ECHOLNPGM(")");
    }
  #endif
}

/**
 * ***************************************************************************
 * ***************************** G-CODE HANDLING *****************************
 * ***************************************************************************
 */

/**
 * Set ABCD destination and feedrate from the current GCode command
 *
 *  - Set destination from included axis codes
 *  - Set to current for missing axis codes
 *  - Set the feedrate, if included
 */
void gcode_get_destination() {
  LOOP_ABCD(i) {
    if (code_seen(axis_codes[i]))
      destination[i] = code_value_axis_units(i) + (axis_relative_modes[i] || relative_mode ? current_position[i] : 0);
    else
      destination[i] = current_position[i];
  }

  if (code_seen('F') && code_value_linear_units() > 0.0)
    feedrate_mm_s = MMM_TO_MMS(code_value_linear_units());
}

void unknown_command_error() {
  SERIAL_ECHO_START;
  SERIAL_ECHOPGM(MSG_UNKNOWN_COMMAND);
  SERIAL_ECHO(current_command);
  SERIAL_ECHOLNPGM("\"");
}

/**
 * G0, G1: Coordinated movement of A B C D axes
 */
inline void gcode_G0_G1() {
  if (IsRunning()) {
    gcode_get_destination(); // Get A B C D F
    prepare_move_to_destination();
  }
}

/**
 * G4: Dwell S<seconds> or P<milliseconds>
 */
inline void gcode_G4() {
  millis_t dwell_ms = 0;

  if (code_seen('P')) dwell_ms = code_value_millis(); // milliseconds to wait
  if (code_seen('S')) dwell_ms = code_value_millis_from_seconds(); // seconds to wait

  stepper.synchronize();
  refresh_cmd_timeout();
  dwell_ms += previous_cmd_ms;  // keep track of when we started waiting

  while (PENDING(millis(), dwell_ms)) idle();
}

/**
 * G28: Home one or more axes
 *
 * Parameters
 *
 *  None  Home ABCD to any endstop
 *
 * Cartesian parameters
 *
 *  A   Home A to the A endstop
 *  B   Home B to the B endstop
 *  C   Home C to the C endstop
 *  D   Home D to the D endstop
 *
 */
inline void gcode_G28() {

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPGM(">>> gcode_G28");
  #endif

  // Wait for planner moves to finish!
  stepper.synchronize();

  setup_for_endstop_move();
  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPGM("> endstops.enable(true)");
  #endif
  endstops.enable(true); // Enable endstops for next homing move

  bool homeA = code_seen('A'), homeB = code_seen('B'), homeC = code_seen('C'), homeD = code_seen('D'),
       home_all_axis = (!homeA && !homeB && !homeC && !homeD) || (homeA && homeB && homeC && homeD);

  set_destination_to_current();

  // Home A
  if (home_all_axis || homeA) {
    HOMEAXIS(A);
    #if ENABLED(DEBUG_LEVELING_FEATURE)
      if (DEBUGGING(LEVELING)) DEBUG_POS("> homeA", current_position);
    #endif
  }

  // Home B
  if (home_all_axis || homeB) {
    HOMEAXIS(B);
    #if ENABLED(DEBUG_LEVELING_FEATURE)
      if (DEBUGGING(LEVELING)) DEBUG_POS("> homeB", current_position);
    #endif
  }

  // Home C
  if (home_all_axis || homeC) {
    HOMEAXIS(C);
    #if ENABLED(DEBUG_LEVELING_FEATURE)
      if (DEBUGGING(LEVELING)) DEBUG_POS("> homeC", current_position);
    #endif
  }

  // Home D
  if (home_all_axis || homeD) {
    HOMEAXIS(D);
    #if ENABLED(DEBUG_LEVELING_FEATURE)
      if (DEBUGGING(LEVELING)) DEBUG_POS("> homeD", current_position);
    #endif
  }

  sync_plan_position();

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPGM("> endstops.not_homing()");
  #endif
  endstops.not_homing();
  endstops.hit_on_purpose(); // clear endstop hit flags

  clean_up_after_endstop_move();

  #if ENABLED(DEBUG_LEVELING_FEATURE)
    if (DEBUGGING(LEVELING)) SERIAL_ECHOLNPGM("<<< gcode_G28");
  #endif

  report_current_position();
}

/**
 * G92: Set current position to given X Y Z E
 */
inline void gcode_G92() {
  LOOP_ABCD(i)
    if (code_seen(axis_codes[i]))
      current_position[i] = code_value_axis_units(i);

  stepper.synchronize();
  sync_plan_position();
}

/**
 * M17: Enable power on all stepper motors
 */
inline void gcode_M17() {
  enable_all_steppers();
}

/**
 * M42: Change pin status via GCode
 *
 *  P<pin>  Pin number (LED if omitted)
 *  S<byte> Pin status from 0 - 255
 */
inline void gcode_M42() {
  if (!code_seen('S')) return;

  int pin_status = code_value_int();
  if (pin_status < 0 || pin_status > 255) return;

  int pin_number = code_seen('P') ? code_value_int() : LED_PIN;
  if (pin_number < 0) return;

  for (uint8_t i = 0; i < COUNT(sensitive_pins); i++)
    if (pin_number == sensitive_pins[i]) return;

  pinMode(pin_number, OUTPUT);
  digitalWrite(pin_number, pin_status);
  analogWrite(pin_number, pin_status);
}

#if DISABLED(EMERGENCY_PARSER)

  /**
   * M112: Emergency Stop
   */
  inline void gcode_M112() { kill(); }


  /**
   * M410: Quickstop - Abort all planned moves
   *
   * This will stop the carriages mid-move, so most likely they
   * will be out of sync with the stepper position after this.
   */
  inline void gcode_M410() { quickstop_stepper(); }

#endif

/**
 * M110: Set Current Line Number
 */
inline void gcode_M110() {
  if (code_seen('N')) gcode_N = code_value_long();
}

/**
 * M111: Set the debug level
 */
inline void gcode_M111() {
  marlin_debug_flags = code_seen('S') ? code_value_byte() : (uint8_t) DEBUG_NONE;

  const static char str_debug_1[] PROGMEM = MSG_DEBUG_ECHO;
  const static char str_debug_2[] PROGMEM = MSG_DEBUG_INFO;
  const static char str_debug_4[] PROGMEM = MSG_DEBUG_ERRORS;
  const static char str_debug_8[] PROGMEM = MSG_DEBUG_DRYRUN;
  const static char str_debug_16[] PROGMEM = MSG_DEBUG_COMMUNICATION;
  #if ENABLED(DEBUG_LEVELING_FEATURE)
    const static char str_debug_32[] PROGMEM = MSG_DEBUG_LEVELING;
  #endif

  const static char* const debug_strings[] PROGMEM = {
    str_debug_1, str_debug_2, str_debug_4, str_debug_8, str_debug_16,
    #if ENABLED(DEBUG_LEVELING_FEATURE)
      str_debug_32
    #endif
  };

  SERIAL_ECHO_START;
  SERIAL_ECHOPGM(MSG_DEBUG_PREFIX);
  if (marlin_debug_flags) {
    uint8_t comma = 0;
    for (uint8_t i = 0; i < COUNT(debug_strings); i++) {
      if (TEST(marlin_debug_flags, i)) {
        if (comma++) SERIAL_CHAR(',');
        serialprintPGM((char*)pgm_read_word(&(debug_strings[i])));
      }
    }
  }
  else {
    SERIAL_ECHOPGM(MSG_DEBUG_OFF);
  }
  SERIAL_EOL;
}

#if HAS_POWER_SWITCH

  /**
   * M80: Turn on Power Supply
   */
  inline void gcode_M80() {
    OUT_WRITE(PS_ON_PIN, PS_ON_AWAKE); //GND

    /**
     * If you have a switch on suicide pin, this is useful
     * if you want to start another print with suicide feature after
     * a print without suicide...
     */
    #if HAS_SUICIDE
      OUT_WRITE(SUICIDE_PIN, HIGH);
    #endif
  }

#endif // HAS_POWER_SWITCH

/**
 * M81: Turn off Power, including Power Supply, if there is one.
 *
 *      This code should ALWAYS be available for EMERGENCY SHUTDOWN!
 */
inline void gcode_M81() {
  stepper.finish_and_disable();
  delay(1000); // Wait 1 second before switching off
  #if HAS_SUICIDE
    stepper.synchronize();
    suicide();
  #elif HAS_POWER_SWITCH
    OUT_WRITE(PS_ON_PIN, PS_ON_ASLEEP);
  #endif
}

/**
 * M18, M84: Disable all stepper motors
 */
inline void gcode_M18_M84() {
  if (code_seen('S')) {
    stepper_inactive_time = code_value_millis_from_seconds();
  }
  else {
    bool all_axis = !((code_seen('A')) || (code_seen('B')) || (code_seen('C')) || (code_seen('D')));
    if (all_axis) {
      stepper.finish_and_disable();
    }
    else {
      stepper.synchronize();
      if (code_seen('A')) disable_a();
      if (code_seen('B')) disable_b();
      if (code_seen('C')) disable_c();
      if (code_seen('D')) disable_d();
    }
  }
}

/**
 * M85: Set inactivity shutdown timer with parameter S<seconds>. To disable set zero (default)
 */
inline void gcode_M85() {
  if (code_seen('S')) max_inactive_time = code_value_millis_from_seconds();
}

/**
 * M92: Set axis steps-per-unit for one or more axes, X, Y, Z, and E.
 *      (Follows the same syntax as G92)
 */
inline void gcode_M92() {
  LOOP_ABCD(i) {
    if (code_seen(axis_codes[i])) {
      if (i == D_AXIS) {
        float value = code_value_per_axis_unit();
        if (value < 20.0) {
          float factor = planner.axis_steps_per_mm[i] / value; // increase e constants if M92 E14 is given for netfab.
          planner.max_feedrate_mm_s[i] *= factor;
          planner.max_acceleration_steps_per_s2[i] *= factor;
        }
        planner.axis_steps_per_mm[i] = value;
      }
      else {
        planner.axis_steps_per_mm[i] = code_value_per_axis_unit();
      }
    }
  }
  planner.refresh_positioning();
}

/**
 * Output the current position to serial
 */
static void report_current_position() {
  SERIAL_PROTOCOLPGM("A:");
  SERIAL_PROTOCOL(current_position[A_AXIS]);
  SERIAL_PROTOCOLPGM(" B:");
  SERIAL_PROTOCOL(current_position[B_AXIS]);
  SERIAL_PROTOCOLPGM(" C:");
  SERIAL_PROTOCOL(current_position[C_AXIS]);
  SERIAL_PROTOCOLPGM(" D:");
  SERIAL_PROTOCOL(current_position[D_AXIS]);

  stepper.report_positions();
}

/**
 * M114: Output current position to serial port
 */
inline void gcode_M114() { report_current_position(); }

/**
 * M115: Capabilities string
 */
inline void gcode_M115() { SERIAL_PROTOCOLPGM(MSG_M115_REPORT); }

/**
 * M119: Output endstop states to serial output
 */
inline void gcode_M119() { endstops.M119(); }

/**
 * M120: Enable endstops and set non-homing endstop state to "enabled"
 */
inline void gcode_M120() { endstops.enable_globally(true); }

/**
 * M121: Disable endstops and set non-homing endstop state to "disabled"
 */
inline void gcode_M121() { endstops.enable_globally(false); }

/**
 * M201: Set max acceleration in units/s^2 for print moves (M201 X1000 Y1000)
 */
inline void gcode_M201() {
  LOOP_ABCD(i) {
    if (code_seen(axis_codes[i])) {
      planner.max_acceleration_mm_per_s2[i] = code_value_axis_units();
    }
  }
  // steps per sq second need to be updated to agree with the units per sq second (as they are what is used in the planner)
  planner.reset_acceleration_rates();
}

/**
 * M203: Set maximum feedrate that your machine can sustain (M203 X200 Y200 Z300 E10000) in units/sec
 */
inline void gcode_M203() {
  LOOP_ABCD(i)
    if (code_seen(axis_codes[i]))
      planner.max_feedrate_mm_s[i] = code_value_axis_units();
}

/**
 * M204: Set Accelerations in units/sec^2 (M204 P1200 R3000 T3000)
 *
 *    P = Printing moves
 *    R = Retract only (no X, Y, Z) moves
 *    T = Travel (non printing) moves
 *
 *  Also sets minimum segment time in ms (B20000) to prevent buffer under-runs and M20 minimum feedrate
 */
inline void gcode_M204() {
  bool pset = code_seen('P');
  if (pset) planner.acceleration = code_value_linear_units();

  bool tset = code_seen('T');
  if (tset) planner.travel_acceleration = code_value_linear_units();

  if (code_seen('S')) {  // Kept for legacy compatibility. Should NOT BE USED for new development.
    planner.travel_acceleration = planner.acceleration = code_value_linear_units();
    pset = tset = true;
  }
  if (pset || (!pset && !tset)) {
    SERIAL_ECHOPAIR("Print Acceleration: ", planner.acceleration);
    SERIAL_EOL;
  }
  if (tset || (!pset && !tset)) {
    SERIAL_ECHOPAIR("Travel Acceleration: ", planner.travel_acceleration);
    SERIAL_EOL;
  }
}

/**
 * M205: Set Advanced Settings
 *
 *    S = Min Feed Rate (units/s)
 *    T = Min Travel Feed Rate (units/s)
 *    B = Min Segment Time (µs)
 *    Z = Max Z Jerk (units/sec^2)
 */
inline void gcode_M205() {
  if (code_seen('S')) planner.min_feedrate_mm_s = code_value_linear_units();
  if (code_seen('T')) planner.min_travel_feedrate_mm_s = code_value_linear_units();
  if (code_seen('B')) planner.min_segment_time = code_value_millis();
  if (code_seen('Z')) planner.max_z_jerk = code_value_axis_units();
}

/**
 * M206: Set Additional Homing Offset (X Y Z). SCARA aliases T=X, P=Y
 */
inline void gcode_M206() {
  LOOP_ABCD(i)
    if (code_seen(axis_codes[i]))
      set_home_offset((AxisEnum)i, code_value_axis_units());

  sync_plan_position();
  report_current_position();
}

/**
 * M220: Set speed percentage factor, aka "Feed Rate" (M220 S95)
 */
inline void gcode_M220() {
  if (code_seen('S')) feedrate_percentage = code_value_int();
}

/**
 * M226: Wait until the specified pin reaches the state required (M226 P<pin> S<state>)
 */
inline void gcode_M226() {
  if (code_seen('P')) {
    int pin_number = code_value_int();

    int pin_state = code_seen('S') ? code_value_int() : -1; // required pin state - default is inverted

    if (pin_state >= -1 && pin_state <= 1) {

      for (uint8_t i = 0; i < COUNT(sensitive_pins); i++) {
        if (sensitive_pins[i] == pin_number) {
          pin_number = -1;
          break;
        }
      }

      if (pin_number > -1) {
        int target = LOW;

        stepper.synchronize();

        pinMode(pin_number, INPUT);

        switch (pin_state) {
          case 1:
            target = HIGH;
            break;

          case 0:
            target = LOW;
            break;

          case -1:
            target = !digitalRead(pin_number);
            break;
        }

        while (digitalRead(pin_number) != target) idle();

      } // pin_number > -1
    } // pin_state -1 0 1
  } // code_seen('P')
}

#if ENABLED(EXPERIMENTAL_I2CBUS)

  /**
   * M260: Send data to a I2C slave device
   *
   * This is a PoC, the formating and arguments for the GCODE will
   * change to be more compatible, the current proposal is:
   *
   *  M260 A<slave device address base 10> ; Sets the I2C slave address the data will be sent to
   *
   *  M260 B<byte-1 value in base 10>
   *  M260 B<byte-2 value in base 10>
   *  M260 B<byte-3 value in base 10>
   *
   *  M260 S1 ; Send the buffered data and reset the buffer
   *  M260 R1 ; Reset the buffer without sending data
   *
   */
  inline void gcode_M260() {
    // Set the target address
    if (code_seen('A')) i2c.address(code_value_byte());

    // Add a new byte to the buffer
    if (code_seen('B')) i2c.addbyte(code_value_byte());

    // Flush the buffer to the bus
    if (code_seen('S')) i2c.send();

    // Reset and rewind the buffer
    else if (code_seen('R')) i2c.reset();
  }

  /**
   * M261: Request X bytes from I2C slave device
   *
   * Usage: M261 A<slave device address base 10> B<number of bytes>
   */
  inline void gcode_M261() {
    if (code_seen('A')) i2c.address(code_value_byte());

    uint8_t bytes = code_seen('B') ? code_value_byte() : 1;

    if (i2c.addr && bytes && bytes <= TWIBUS_BUFFER_SIZE) {
      i2c.relay(bytes);
    }
    else {
      SERIAL_ERROR_START;
      SERIAL_ERRORLN("Bad i2c request");
    }
  }

#endif // EXPERIMENTAL_I2CBUS

#if HAS_BUZZER

  /**
   * M300: Play beep sound S<frequency Hz> P<duration ms>
   */
  inline void gcode_M300() {
    uint16_t const frequency = code_seen('S') ? code_value_ushort() : 260;
    uint16_t duration = code_seen('P') ? code_value_ushort() : 1000;

    // Limits the tone duration to 0-5 seconds.
    NOMORE(duration, 5000);

    BUZZ(duration, frequency);
  }

#endif // HAS_BUZZER

/**
 * M400: Finish all moves
 */
inline void gcode_M400() { stepper.synchronize(); }

void quickstop_stepper() {
  stepper.quick_stop();
  stepper.synchronize();
  LOOP_ABCD(i) set_current_from_steppers_for_axis((AxisEnum)i);
  sync_plan_position();
}

/**
 * M500: Store settings in EEPROM
 */
inline void gcode_M500() { Config_StoreSettings(); }

/**
 * M501: Read settings from EEPROM
 */
inline void gcode_M501() { Config_RetrieveSettings(); }

/**
 * M502: Revert to default settings
 */
inline void gcode_M502() { Config_ResetDefault(); }

/**
 * M503: print settings currently in memory
 */
inline void gcode_M503() { Config_PrintSettings(code_seen('S') && !code_value_bool()); }

/**
 * M907: Set digital trimpot motor current using axis codes X, Y, Z, E, B, S
 */
inline void gcode_M907() {
  #if HAS_DIGIPOTSS
    LOOP_ABCD(i)
      if (code_seen(axis_codes[i])) stepper.digipot_current(i, code_value_int());
    if (code_seen('B')) stepper.digipot_current(4, code_value_int());
    if (code_seen('S')) for (int i = 0; i <= 4; i++) stepper.digipot_current(i, code_value_int());
  #endif
  #if PIN_EXISTS(MOTOR_CURRENT_PWM_XY)
    if (code_seen('A')) stepper.digipot_current(0, code_value_int());
  #endif
  #if PIN_EXISTS(MOTOR_CURRENT_PWM_Z)
    if (code_seen('C')) stepper.digipot_current(1, code_value_int());
  #endif
  #if PIN_EXISTS(MOTOR_CURRENT_PWM_E)
    if (code_seen('D')) stepper.digipot_current(2, code_value_int());
  #endif
  #if ENABLED(DIGIPOT_I2C)
    // this one uses actual amps in floating point
    LOOP_ABCD(i) if (code_seen(axis_codes[i])) digipot_i2c_set_current(i, code_value_float());
    // for each additional extruder (named B,C,D,E..., channels 4,5,6,7...)
    for (int i = NUM_AXIS; i < DIGIPOT_I2C_NUM_CHANNELS; i++) if (code_seen('B' + i - (NUM_AXIS))) digipot_i2c_set_current(i, code_value_float());
  #endif
  #if ENABLED(DAC_STEPPER_CURRENT)
    if (code_seen('S')) {
      float dac_percent = code_value_float();
      for (uint8_t i = 0; i <= 4; i++) dac_current_percent(i, dac_percent);
    }
    LOOP_ABCD(i) if (code_seen(axis_codes[i])) dac_current_percent(i, code_value_float());
  #endif
}

#if HAS_DIGIPOTSS || ENABLED(DAC_STEPPER_CURRENT)

  /**
   * M908: Control digital trimpot directly (M908 P<pin> S<current>)
   */
  inline void gcode_M908() {
    #if HAS_DIGIPOTSS
      stepper.digitalPotWrite(
        code_seen('P') ? code_value_int() : 0,
        code_seen('S') ? code_value_int() : 0
      );
    #endif
    #ifdef DAC_STEPPER_CURRENT
      dac_current_raw(
        code_seen('P') ? code_value_byte() : -1,
        code_seen('S') ? code_value_ushort() : 0
      );
    #endif
  }

  #if ENABLED(DAC_STEPPER_CURRENT) // As with Printrbot RevF

    inline void gcode_M909() { dac_print_values(); }

    inline void gcode_M910() { dac_commit_eeprom(); }

  #endif

#endif // HAS_DIGIPOTSS || DAC_STEPPER_CURRENT

#if HAS_MICROSTEPS

  // M350 Set microstepping mode. Warning: Steps per unit remains unchanged. S code sets stepping mode for all drivers.
  inline void gcode_M350() {
    if (code_seen('S')) for (int i = 0; i <= 4; i++) stepper.microstep_mode(i, code_value_byte());
    LOOP_ABCD(i) if (code_seen(axis_codes[i])) stepper.microstep_mode(i, code_value_byte());
    if (code_seen('B')) stepper.microstep_mode(4, code_value_byte());
    stepper.microstep_readings();
  }

  /**
   * M351: Toggle MS1 MS2 pins directly with axis codes X Y Z E B
   *       S# determines MS1 or MS2, X# sets the pin high/low.
   */
  inline void gcode_M351() {
    if (code_seen('S')) switch (code_value_byte()) {
      case 1:
        LOOP_ABCD(i) if (code_seen(axis_codes[i])) stepper.microstep_ms(i, code_value_byte(), -1);
        if (code_seen('B')) stepper.microstep_ms(4, code_value_byte(), -1);
        break;
      case 2:
        LOOP_ABCD(i) if (code_seen(axis_codes[i])) stepper.microstep_ms(i, -1, code_value_byte());
        if (code_seen('B')) stepper.microstep_ms(4, -1, code_value_byte());
        break;
    }
    stepper.microstep_readings();
  }

#endif // HAS_MICROSTEPS

/**
 * M999: Restart after being stopped
 *
 * Default behaviour is to flush the serial buffer and request
 * a resend to the host starting on the last N line received.
 *
 * Sending "M999 S1" will resume printing without flushing the
 * existing command buffer.
 *
 */
inline void gcode_M999() {
  Running = true;
  if (code_seen('S') && code_value_bool()) return;
  FlushSerialRequestResend();
}

/**
 * Process a single command and dispatch it to its handler
 * This is called from the main loop()
 */
void process_next_command() {
  current_command = command_queue[cmd_queue_index_r];

  if (DEBUGGING(ECHO)) {
    SERIAL_ECHO_START;
    SERIAL_ECHOLN(current_command);
  }

  // Sanitize the current command:
  //  - Skip leading spaces
  //  - Bypass N[-0-9][0-9]*[ ]*
  //  - Overwrite * with nul to mark the end
  while (*current_command == ' ') ++current_command;
  if (*current_command == 'N' && NUMERIC_SIGNED(current_command[1])) {
    current_command += 2; // skip N[-0-9]
    while (NUMERIC(*current_command)) ++current_command; // skip [0-9]*
    while (*current_command == ' ') ++current_command; // skip [ ]*
  }
  char* starpos = strchr(current_command, '*');  // * should always be the last parameter
  if (starpos) while (*starpos == ' ' || *starpos == '*') *starpos-- = '\0'; // nullify '*' and ' '

  char *cmd_ptr = current_command;

  // Get the command code, which must be G, M, or T
  char command_code = *cmd_ptr++;

  // Skip spaces to get the numeric part
  while (*cmd_ptr == ' ') cmd_ptr++;

  uint16_t codenum = 0; // define ahead of goto

  // Bail early if there's no code
  bool code_is_good = NUMERIC(*cmd_ptr);
  if (!code_is_good) goto ExitUnknownCommand;

  // Get and skip the code number
  do {
    codenum = (codenum * 10) + (*cmd_ptr - '0');
    cmd_ptr++;
  } while (NUMERIC(*cmd_ptr));

  // Skip all spaces to get to the first argument, or nul
  while (*cmd_ptr == ' ') cmd_ptr++;

  // The command's arguments (if any) start here, for sure!
  current_command_args = cmd_ptr;

  // Handle a known G, M, or T
  switch (command_code) {
    case 'G': switch (codenum) {

      // G0, G1
      case 0:
      case 1:
        gcode_G0_G1();
        break;

      // G4 Dwell
      case 4:
        gcode_G4();
        break;

      case 28: // G28: Home all axes, one at a time
        gcode_G28();
        break;

      case 90: // G90
        relative_mode = false;
        break;
      case 91: // G91
        relative_mode = true;
        break;

      case 92: // G92
        gcode_G92();
        break;
    }
    break;

    case 'M': switch (codenum) {

      case 17:
        gcode_M17();
        break;

      case 42: //M42 -Change pin status via gcode
        gcode_M42();
        break;

      #if ENABLED(M100_FREE_MEMORY_WATCHER)
        case 100:
          gcode_M100();
          break;
      #endif

      case 110: // M110: Set Current Line Number
        gcode_M110();
        break;

      case 111: // M111: Set debug level
        gcode_M111();
        break;

      #if DISABLED(EMERGENCY_PARSER)

        case 112: // M112: Emergency Stop
          gcode_M112();
          break;

        case 410: // M410 quickstop - Abort all the planned moves.
          gcode_M410();
          break;

      #endif

      #if HAS_POWER_SWITCH

        case 80: // M80: Turn on Power Supply
          gcode_M80();
          break;

      #endif // HAS_POWER_SWITCH

      case 81: // M81: Turn off Power, including Power Supply, if possible
        gcode_M81();
        break;

      case 18: // (for compatibility)
      case 84: // M84
        gcode_M18_M84();
        break;
      case 85: // M85
        gcode_M85();
        break;
      case 92: // M92: Set the steps-per-unit for one or more axes
        gcode_M92();
        break;
      case 115: // M115: Report capabilities
        gcode_M115();
        break;
      case 114: // M114: Report current position
        gcode_M114();
        break;
      case 120: // M120: Enable endstops
        gcode_M120();
        break;
      case 121: // M121: Disable endstops
        gcode_M121();
        break;
      case 119: // M119: Report endstop states
        gcode_M119();
        break;

      case 203: // M203 max feedrate units/sec
        gcode_M203();
        break;
      case 204: // M204 acceleration S normal moves T filmanent only moves
        gcode_M204();
        break;
      case 205: //M205 advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk
        gcode_M205();
        break;
      case 206: // M206 additional homing offset
        gcode_M206();
        break;

      case 220: // M220 - Set Feedrate Percentage: S<percent> ("FR" on your LCD)
        gcode_M220();
        break;

      case 226: // M226 P<pin number> S<pin state>- Wait until the specified pin reaches the state required
        gcode_M226();
        break;

      #if ENABLED(EXPERIMENTAL_I2CBUS)

        case 260: // M260: Send data to an i2c slave
          gcode_M260();
          break;

        case 261: // M261: Request data from an i2c slave
          gcode_M261();
          break;

      #endif // EXPERIMENTAL_I2CBUS

      #if HAS_BUZZER
        case 300: // M300 - Play beep tone
          gcode_M300();
          break;
      #endif // HAS_BUZZER

      case 400: // M400 finish all moves
        gcode_M400();
        break;

      case 500: // M500 Store settings in EEPROM
        gcode_M500();
        break;
      case 501: // M501 Read settings from EEPROM
        gcode_M501();
        break;
      case 502: // M502 Revert to default settings
        gcode_M502();
        break;
      case 503: // M503 print settings currently in memory
        gcode_M503();
        break;

      case 907: // M907 Set digital trimpot motor current using axis codes.
        gcode_M907();
        break;

      #if HAS_DIGIPOTSS || ENABLED(DAC_STEPPER_CURRENT)

        case 908: // M908 Control digital trimpot directly.
          gcode_M908();
          break;

        #if ENABLED(DAC_STEPPER_CURRENT) // As with Printrbot RevF

          case 909: // M909 Print digipot/DAC current value
            gcode_M909();
            break;

          case 910: // M910 Commit digipot/DAC value to external EEPROM
            gcode_M910();
            break;

        #endif

      #endif // HAS_DIGIPOTSS || DAC_STEPPER_CURRENT

      #if HAS_MICROSTEPS

        case 350: // M350 Set microstepping mode. Warning: Steps per unit remains unchanged. S code sets stepping mode for all drivers.
          gcode_M350();
          break;

        case 351: // M351 Toggle MS1 MS2 pins directly, S# determines MS1 or MS2, X# sets the pin high/low.
          gcode_M351();
          break;

      #endif // HAS_MICROSTEPS

      case 999: // M999: Restart after being Stopped
        gcode_M999();
        break;
    }
    break;

    default: code_is_good = false;
  }

ExitUnknownCommand:

  // Still unknown command? Throw an error
  if (!code_is_good) unknown_command_error();

  ok_to_send();
}

void FlushSerialRequestResend() {
  //char command_queue[cmd_queue_index_r][100]="Resend:";
  MYSERIAL.flush();
  SERIAL_PROTOCOLPGM(MSG_RESEND);
  SERIAL_PROTOCOLLN(gcode_LastN + 1);
  ok_to_send();
}

void ok_to_send() {
  refresh_cmd_timeout();
  if (!send_ok[cmd_queue_index_r]) return;
  SERIAL_PROTOCOLPGM(MSG_OK);
  #if ENABLED(ADVANCED_OK)
    char* p = command_queue[cmd_queue_index_r];
    if (*p == 'N') {
      SERIAL_PROTOCOL(' ');
      SERIAL_ECHO(*p++);
      while (NUMERIC_SIGNED(*p))
        SERIAL_ECHO(*p++);
    }
    SERIAL_PROTOCOLPGM(" P"); SERIAL_PROTOCOL(int(BLOCK_BUFFER_SIZE - planner.movesplanned() - 1));
    SERIAL_PROTOCOLPGM(" B"); SERIAL_PROTOCOL(BUFSIZE - commands_in_queue);
  #endif
  SERIAL_EOL;
}

void set_current_from_steppers_for_axis(AxisEnum axis) {
  current_position[axis] = stepper.get_axis_position_mm(axis); // CORE handled transparently
}

/**
 * Prepare a single move and get ready for the next one
 *
 * (This may call planner.buffer_line several times to put
 *  smaller moves into the planner for DELTA or SCARA.)
 */
void prepare_move_to_destination() {
  refresh_cmd_timeout();
  line_to_destination(MMS_SCALED(feedrate_mm_s));
  set_current_to_destination();
}

#if HAS_CONTROLLERFAN

  void controllerFan() {
    static millis_t lastMotorOn = 0; // Last time a motor was turned on
    static millis_t nextMotorCheck = 0; // Last time the state was checked
    millis_t ms = millis();
    if (ELAPSED(ms, nextMotorCheck)) {
      nextMotorCheck = ms + 2500UL; // Not a time critical function, so only check every 2.5s
      if (A_ENABLE_READ == A_ENABLE_ON || B_ENABLE_READ == B_ENABLE_ON || C_ENABLE_READ == C_ENABLE_ON || D_ENABLE_READ == D_ENABLE_ON) {
        lastMotorOn = ms; //... set time to NOW so the fan will turn on
      }

      // Fan off if no steppers have been enabled for CONTROLLERFAN_SECS seconds
      uint8_t speed = (!lastMotorOn || ELAPSED(ms, lastMotorOn + (CONTROLLERFAN_SECS) * 1000UL)) ? 0 : CONTROLLERFAN_SPEED;

      // allows digital or PWM fan output to be used (see M42 handling)
      digitalWrite(CONTROLLERFAN_PIN, speed);
      analogWrite(CONTROLLERFAN_PIN, speed);
    }
  }

#endif // HAS_CONTROLLERFAN

void enable_all_steppers() {
  enable_a();
  enable_b();
  enable_c();
  enable_d();
}

void disable_all_steppers() {
  disable_a();
  disable_b();
  disable_c();
  disable_d();
}

/**
 * Standard idle routine keeps the machine alive
 */
void idle() {
  manage_inactivity();
  #if HAS_BUZZER && PIN_EXISTS(BEEPER)
    buzzer.tick();
  #endif
}

/**
 * Manage several activities:
 *  - Check for Filament Runout
 *  - Keep the command buffer full
 *  - Check for maximum inactive time between commands
 *  - Check for maximum inactive time between stepper commands
 *  - Check if pin CHDK needs to go LOW
 *  - Check for KILL button held down
 *  - Check for HOME button held down
 *  - Check if cooling fan needs to be switched on
 *  - Check if an idle but hot extruder needs filament extruded (EXTRUDER_RUNOUT_PREVENT)
 */
void manage_inactivity(bool ignore_stepper_queue/*=false*/) {

  if (commands_in_queue < BUFSIZE) get_available_commands();

  millis_t ms = millis();

  if (max_inactive_time && ELAPSED(ms, previous_cmd_ms + max_inactive_time)) kill();

  if (stepper_inactive_time && ELAPSED(ms, previous_cmd_ms + stepper_inactive_time)
      && !ignore_stepper_queue && !planner.blocks_queued()) {
    #if ENABLED(DISABLE_INACTIVE_A)
      disable_a();
    #endif
    #if ENABLED(DISABLE_INACTIVE_B)
      disable_b();
    #endif
    #if ENABLED(DISABLE_INACTIVE_C)
      disable_c();
    #endif
    #if ENABLED(DISABLE_INACTIVE_D)
      disable_d();
    #endif
  }

  #if HAS_KILL

    // Check if the kill button was pressed and wait just in case it was an accidental
    // key kill key press
    // -------------------------------------------------------------------------------
    static int killCount = 0;   // make the inactivity button a bit less responsive
    const int KILL_DELAY = 750;
    if (!READ(KILL_PIN))
      killCount++;
    else if (killCount > 0)
      killCount--;

    // Exceeded threshold and we can confirm that it was not accidental
    // KILL the machine
    // ----------------------------------------------------------------
    if (killCount >= KILL_DELAY) kill();
  #endif

  #if HAS_CONTROLLERFAN
    controllerFan(); // Check if fan should be turned on to cool stepper drivers down
  #endif

  planner.check_axes_activity();
}

void kill() {
  SERIAL_ERROR_START;
  SERIAL_ERRORLNPGM(MSG_ERR_KILLED);

  delay(500); // Wait a short time

  cli(); // Stop interrupts

  disable_all_steppers();

  #if HAS_POWER_SWITCH
    pinMode(PS_ON_PIN, INPUT);
  #endif

  suicide();
  while (1) { } // Wait for reset
}

void stop() {
  if (IsRunning()) {
    Running = false;
    Stopped_gcode_LastN = gcode_LastN; // Save last g_code for restart
    SERIAL_ERROR_START;
    SERIAL_ERRORLNPGM(MSG_ERR_STOPPED);
  }
}
