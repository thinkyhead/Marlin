/**
 * TEMPORARY bring-up diagnostics for the AnkerMake M5 nozzle board. NOT FOR MERGE.
 *
 * M7700 [S<seconds>] [T<threshold>] [H]
 *   Listen to the nozzle board on each candidate UART4 pin pair and report:
 *   - bytes received, GCP frames (AA 55) seen and their command IDs,
 *   - the first bytes in hex,
 *   - PB0 (probe) level changes during the window (tap the nozzle to test).
 *   S  Seconds per pin pair (default 4)
 *   H  Also send the 100 ms heartbeat (target 0 °C, fan 0) so the board sees a host
 *   T  Send PROBE_SET (0x21) with this threshold (e.g. T800) every 100 ms for the first 2 s
 *   P  Only try pin pair P (0..2)
 */
#include "../inc/MarlinConfig.h"

#if MB(ANKERMAKE_V8111)

#include "gcode.h"
#include "../MarlinCore.h"
#include <HardwareSerial.h>

static void hexb(const uint8_t b) {
  static const char hx[] = "0123456789ABCDEF";
  SERIAL_CHAR(hx[b >> 4], hx[b & 0xF]);
}

static void gcp_send(HardwareSerial &ser, const uint8_t type, const uint8_t cmd, const uint8_t *data, const uint16_t len) {
  uint8_t h[8] = { 0xAA, 0x55, uint8_t(len & 0xFF), uint8_t(len >> 8), type, 0, 0x21, cmd };
  uint8_t sum = 0;
  for (uint8_t i = 0; i < 8; ++i) if (i != 5) sum += h[i];
  for (uint16_t i = 0; i < len; ++i) sum += data[i];
  h[5] = sum;
  ser.write(h, 8);
  if (len) ser.write(data, len);
}

void GcodeSuite::M7700() {
  static const struct { uint32_t rx, tx; const char *name; } cand[] = {
    { PA1,  PC10, "RX PA1 / TX PC10" },
    { PC11, PC10, "RX PC11 / TX PC10" },
    { PA1,  PA0,  "RX PA1 / TX PA0" }
  };
  const millis_t window = SEC_TO_MS(parser.ushortval('S', 4));
  const bool heartbeat = parser.seen_test('H');
  const int16_t thr = parser.intval('T', 0);
  const int8_t only = parser.intval('P', -1);

  SET_INPUT_PULLUP(PB0);
  SERIAL_ECHOLNPGM("M7700 PB0=", READ(PB0), " PC6=", READ(PC6), " PC5(pwr)=", READ(PC5));

  for (uint8_t c = 0; c < COUNT(cand); ++c) {
    if (only >= 0 && only != c) continue;
    HardwareSerial ser(cand[c].rx, cand[c].tx);
    ser.begin(115200);

    uint32_t bytes = 0, frames = 0;
    uint8_t first[48], nfirst = 0, prev = 0, cmds[32], ncmds = 0;
    int8_t state = -1;  // -1: no frame, 0..7 header byte index
    uint8_t pb0 = READ(PB0), pb0_changes = 0;
    millis_t pb0_first_change = 0;
    const millis_t start = millis();
    millis_t next_tx = start;

    while (PENDING(millis(), start + window)) {
      const millis_t now = millis();
      if (ELAPSED(now, next_tx)) {
        next_tx = now + 100;
        if (heartbeat) {
          const uint8_t hb[5] = { 1, 0, 0, 0, 0 };
          gcp_send(ser, 5, 0x20, hb, 5);
        }
        if (thr && now - start < 2000) {
          const uint8_t d[2] = { uint8_t(thr & 0xFF), uint8_t(uint16_t(thr) >> 8) };
          gcp_send(ser, 2, 0x21, d, 2);
        }
      }
      while (ser.available()) {
        const uint8_t b = ser.read();
        ++bytes;
        if (nfirst < COUNT(first)) first[nfirst++] = b;
        // Count AA 55 headers and grab the command byte (offset 7)
        if (prev == 0xAA && b == 0x55) { ++frames; state = 1; }
        else if (state >= 1) {
          ++state;
          if (state == 7) { if (ncmds < COUNT(cmds)) cmds[ncmds++] = b; state = -1; }
        }
        prev = b;
      }
      const uint8_t p = READ(PB0);
      if (p != pb0) { pb0 = p; if (!pb0_changes++) pb0_first_change = now - start; }
      hal.watchdog_refresh();
      marlin.idle_no_sleep();
    }
    ser.end();

    SERIAL_ECHOLNPGM("M7700 ", cand[c].name, ": bytes=", bytes, " frames=", frames,
                     " PB0 changes=", pb0_changes, " (first at ", pb0_first_change, " ms) PB0 now=", pb0);
    if (ncmds) {
      SERIAL_ECHOPGM("M7700  cmds:");
      for (uint8_t i = 0; i < ncmds; ++i) { SERIAL_CHAR(' '); hexb(cmds[i]); }
      SERIAL_EOL();
    }
    if (nfirst) {
      SERIAL_ECHOPGM("M7700  hex:");
      for (uint8_t i = 0; i < nfirst; ++i) { SERIAL_CHAR(' '); hexb(first[i]); }
      SERIAL_EOL();
    }
  }
}

#endif // ANKERMAKE_V8111
