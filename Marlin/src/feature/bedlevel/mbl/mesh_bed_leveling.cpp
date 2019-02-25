/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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

#include "../../../inc/MarlinConfig.h"

#if ENABLED(MESH_BED_LEVELING)

  #include "mesh_bed_leveling.h"

  #include "../../../module/motion.h"
  #include "../../../feature/bedlevel/bedlevel.h"

  mesh_bed_leveling mbl;

  float mesh_bed_leveling::z_offset,
        mesh_bed_leveling::z_values[GRID_MAX_POINTS_X][GRID_MAX_POINTS_Y],
        mesh_bed_leveling::index_to_xpos[GRID_MAX_POINTS_X],
        mesh_bed_leveling::index_to_ypos[GRID_MAX_POINTS_Y];

  mesh_bed_leveling::mesh_bed_leveling() {
    for (uint8_t i = 0; i < GRID_MAX_POINTS_X; ++i)
      index_to_xpos[i] = MESH_MIN_X + i * (MESH_X_DIST);
    for (uint8_t i = 0; i < GRID_MAX_POINTS_Y; ++i)
      index_to_ypos[i] = MESH_MIN_Y + i * (MESH_Y_DIST);
    reset();
  }

  void mesh_bed_leveling::reset() {
    z_offset = 0;
    ZERO(z_values);
  }

  #if IS_CARTESIAN && DISABLED(SEGMENT_LEVELED_MOVES)

    /**
     * Prepare a mesh-leveled linear move in a Cartesian setup,
     * splitting the move where it crosses mesh borders.
     */
    void mesh_bed_leveling::line_to_destination(const float fr_mm_s, uint8_t x_splits, uint8_t y_splits) {
      // Get current and destination cells for this line
      int cx1 = cell_index_x(tool.position[X_AXIS]),
          cy1 = cell_index_y(tool.position[Y_AXIS]),
          cx2 = cell_index_x(tool.destination[X_AXIS]),
          cy2 = cell_index_y(tool.destination[Y_AXIS]);
      NOMORE(cx1, GRID_MAX_POINTS_X - 2);
      NOMORE(cy1, GRID_MAX_POINTS_Y - 2);
      NOMORE(cx2, GRID_MAX_POINTS_X - 2);
      NOMORE(cy2, GRID_MAX_POINTS_Y - 2);

      // Start and end in the same cell? No split needed.
      if (cx1 == cx2 && cy1 == cy2) {
        line_to_destination(fr_mm_s);
        tool.set_position_from_destination();
        return;
      }

      #define MBL_SEGMENT_END(A) (tool.position[_AXIS(A)] + (tool.destination[_AXIS(A)] - tool.position[_AXIS(A)]) * normalized_dist)

      float normalized_dist, end[XYZE];
      const int8_t gcx = MAX(cx1, cx2), gcy = MAX(cy1, cy2);

      // Crosses on the X and not already split on this X?
      // The x_splits flags are insurance against rounding errors.
      if (cx2 != cx1 && TEST(x_splits, gcx)) {
        // Split on the X grid line
        CBI(x_splits, gcx);
        COPY(end, tool.destination);
        tool.destination[X_AXIS] = index_to_xpos[gcx];
        normalized_dist = (tool.destination[X_AXIS] - tool.position[X_AXIS]) / (end[X_AXIS] - tool.position[X_AXIS]);
        tool.destination[Y_AXIS] = MBL_SEGMENT_END(Y);
      }
      // Crosses on the Y and not already split on this Y?
      else if (cy2 != cy1 && TEST(y_splits, gcy)) {
        // Split on the Y grid line
        CBI(y_splits, gcy);
        COPY(end, tool.destination);
        tool.destination[Y_AXIS] = index_to_ypos[gcy];
        normalized_dist = (tool.destination[Y_AXIS] - tool.position[Y_AXIS]) / (end[Y_AXIS] - tool.position[Y_AXIS]);
        tool.destination[X_AXIS] = MBL_SEGMENT_END(X);
      }
      else {
        // Must already have been split on these border(s)
        // This should be a rare case.
        line_to_destination(fr_mm_s);
        tool.set_position_from_destination();
        return;
      }

      tool.destination[Z_AXIS] = MBL_SEGMENT_END(Z);
      tool.destination[E_AXIS] = MBL_SEGMENT_END(E);

      // Do the split and look for more borders
      line_to_destination(fr_mm_s, x_splits, y_splits);

      // Restore destination from stack
      COPY(tool.destination, end);
      line_to_destination(fr_mm_s, x_splits, y_splits);
    }

  #endif // IS_CARTESIAN && !SEGMENT_LEVELED_MOVES

  void mesh_bed_leveling::report_mesh() {
    SERIAL_ECHOPAIR_F(STRINGIFY(GRID_MAX_POINTS_X) "x" STRINGIFY(GRID_MAX_POINTS_Y) " mesh. Z offset: ", z_offset, 5);
    SERIAL_ECHOLNPGM("\nMeasured points:");
    print_2d_array(GRID_MAX_POINTS_X, GRID_MAX_POINTS_Y, 5,
      [](const uint8_t ix, const uint8_t iy) { return z_values[ix][iy]; }
    );
  }

#endif // MESH_BED_LEVELING
