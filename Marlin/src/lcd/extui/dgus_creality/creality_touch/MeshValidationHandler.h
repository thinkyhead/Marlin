/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

class MeshValidationHandler {
  public:
    static void Init();

    static void HandleTemperature(DGUS_VP_Variable &var, void *val_ptr);

    static void HandleStartOrCancelButton(DGUS_VP_Variable &var, void *val_ptr);

    static void OnMeshValidationStart();
    static void OnMeshValidationFinish();

  public:
    static celsius_t nozzle_temperature;
    static celsius_t bed_temperature;

    static feedRate_t prev_feedrate;

    static bool is_cancelling;
    static bool is_running;
    static bool started_from_screen;

  private:
    static void Start();
    static void Cancel();

    static void ValidateTemperatures();
    static void SetStatusMessage(PGM_P statusMessage);
};

constexpr feedRate_t MESH_VALIDATION_PATTERN_FEEDRATE = G26_XY_FEEDRATE;
