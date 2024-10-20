#include "../../inc/MarlinConfig.h"

#if ENABLED(HANDSHAKE)

#include "handshake.h"

HandShake hand_shake;

void HandShake::init() {
  pinMode(HANDSHAKE_SDO, INPUT_FLOATING);
  SET_OUTPUT(HEATER_EN_PIN);
  WRITE(HEATER_EN_PIN, !HEATER_EN_STATE);
  if (READ(HANDSHAKE_SDO))
    WRITE(HEATER_EN_PIN, HEATER_EN_STATE);
  else
    SERIAL_ECHOLNPGM("echo:The nozzle is connected incorrectly");
}

void HandShake::handshake_test() {
  //u8 status=0;
  //switch (status) {
  //  case 0: break;
  //  case 1: break;
  //  case 2: break;
  //  default: break;
  //}
  //
  //SET_INPUT(HANDSHAKE_SDO);
  //WRITE(HANDSHAKE_SDO, !HANDSHAKE_STATE);
  //WRITE(PROBE_CONTROL_PIN, PROBE_CONTROL_STATE); // open power
  //for (uint16_t time = 0; time < 20; time++) {
  //  WRITE(HANDSHAKE_SDO, !HANDSHAKE_STATE);
  //  _delay_ms(10);
  //  WRITE(HANDSHAKE_SDO, HANDSHAKE_STATE);
  //  _delay_ms(10);
  //}
  //SET_INPUT(HANDSHAKE_SDO);
}

void HandShake::check() {
  millis_t ms = millis();
  static millis_t next_report_ms = millis() + HANDSHAKE_TIME;

  ms = millis();
  if (ELAPSED(ms, next_report_ms)) {
    next_report_ms = ms + HANDSHAKE_TIME;
    pinMode(HANDSHAKE_SDO, INPUT_FLOATING);
    if (READ(HANDSHAKE_SDO))
      WRITE(HEATER_EN_PIN, HEATER_EN_STATE);
    else {
      WRITE(HEATER_EN_PIN, !HEATER_EN_STATE);
      //SERIAL_ECHO("echo:The nozzle is connected incorrectly");
      SERIAL_ECHOLNPAIR("echo:The nozzle:", int(READ(HANDSHAKE_SDO)));
    }
  }
}

#endif // HANDSHAKE
