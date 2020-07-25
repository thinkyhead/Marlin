#pragma once

#include <fstream>

#include "window.h"
#include "user_interface.h"

#include "hardware/Heater.h"
#include "hardware/ST7920Device.h"

#include "visualisation.h"
#include "src/inc/MarlinConfig.h"

class Simulation {
public:

  Simulation() :  hotend(HEATER_0_PIN, TEMP_0_PIN),
                  bed(HEATER_BED_PIN, TEMP_BED_PIN),
                  display(LCD_PINS_D4, LCD_PINS_ENABLE, LCD_PINS_RS, BEEPER_PIN, BTN_EN1, BTN_EN2, BTN_ENC, KILL_PIN) {}

  void process_event(SDL_Event& e) {}

  void update() {
    hotend.update();
    bed.update();
    display.update();
  }

  Heater hotend;
  Heater bed;
  ST7920Device display;
  Visualisation vis;
};

class Application {
public:
  Application();
  ~Application();

  void update();
  void render();

  bool active = true;
  Window window;
  UserInterface user_interface;
  Simulation sim;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  std::ifstream input_file;
};