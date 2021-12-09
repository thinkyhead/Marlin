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

class EstepsHandler {
  public:
    static void Init();

    static void HandleStartButton(DGUS_VP_Variable &var, void *val_ptr);
    static void HandleApplyButton(DGUS_VP_Variable &var, void *val_ptr);
    static void HandleBackButton(DGUS_VP_Variable &var, void *val_ptr);

    static void HandleRemainingFilament(DGUS_VP_Variable &var, void *val_ptr);

  public:
    static float set_esteps;
    static float calculated_esteps;

    static float mark_filament_mm;
    static float remaining_filament;

    static float filament_to_extrude;
    static celsius_t calibration_temperature;

  private:
    static void SaveSettingsAndReturn(bool fullConfirm);
    static void SetStatusMessage(PGM_P statusMessage);
};
