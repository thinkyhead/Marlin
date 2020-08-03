#ifdef __PLAT_NATIVE_SIM__

#include <mutex>
#include <fstream>
#include <cmath>
#include <random>
#include "Gpio.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include "ST7920Device.h"

ST7920Device::ST7920Device(pin_type clk, pin_type mosi, pin_type cs,  pin_type beeper, pin_type enc1, pin_type enc2, pin_type enc_but, pin_type kill)
  : clk_pin(clk), mosi_pin(mosi), cs_pin(cs), beeper_pin(beeper), enc1_pin(enc1), enc2_pin(enc2), enc_but_pin(enc_but), kill_pin(kill) {

  Gpio::attach(clk_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(cs_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(beeper_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(kill_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(enc_but_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(enc1_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(enc2_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));

  glGenTextures(1, &texture_id);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
}

ST7920Device::~ST7920Device() {}

void ST7920Device::process_command(Command cmd) {
  if (cmd.rs) {
    graphic_ram[coordinate[1] + (coordinate[0] * (256 / 8))] = cmd.data;
    if (++coordinate[1] > 32) {
      coordinate[1] = 0;
    }
    dirty = true;

  } else if (extended_instruction_set) {
    if (cmd.data & (1 << 7)) {
      // cmd [7] SET GRAPHICS RAM COORDINATE
      coordinate[coordinate_index++] = cmd.data & 0x7F;
      if(coordinate_index == 2) {
        coordinate_index = 0;
        coordinate[1] *= 2;
        if (coordinate[1] >= 128 / 8) {
          coordinate[1] = 0;
          coordinate[0] += 32;
        }
      }
    } else if (cmd.data & (1 << 6)) {
      //printf("cmd: [6] SET IRAM OR SCROLL ADDRESS (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 5)) {
      extended_instruction_set = cmd.data & 0b100;
      //printf("cmd: [5] EXTENDED FUNCTION SET (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 4)) {
      //printf("cmd: [4] UNKNOWN? (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 3)) {
      //printf("cmd: [3] DISPLAY STATUS (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 2)) {
      //printf("cmd: [2] REVERSE (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 1)) {
      //printf("cmd: [1] VERTICAL SCROLL OR RAM ADDRESS SELECT (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 0)) {
      //printf("cmd: [0] STAND BY\n");
    }
  } else {
    if (cmd.data & (1 << 7)) {
      //printf("cmd: [7] SET DDRAM ADDRESS (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 6)) {
      //printf("cmd: [6] SET CGRAM ADDRESS (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 5)) {
      extended_instruction_set = cmd.data & 0b100;
      //printf("cmd: [5] FUNCTION SET (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 4)) {
      //printf("cmd: [4] CURSOR DISPLAY CONTROL (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 3)) {
      //printf("cmd: [3] DISPLAY CONTROL (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 2)) {
      address_increment = cmd.data & 0x1;
      //printf("cmd: [2] ENTRY MODE SET (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 1)) {
      //printf("cmd: [1] RETURN HOME (0x%02X)\n", cmd.data);
    } else if (cmd.data & (1 << 0)) {
      //printf("cmd: [0] DISPLAY CLEAR\n");
    }
  }
}

void ST7920Device::update() {
  static uint8_t buffer[256*64*3] = {};
  auto now = clock.now();
  float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_update).count();

  if (dirty && delta > 1.0 / 30.0) {
    last_update = now;
    for (std::size_t x = 0; x < 128; x++) {
      for (std::size_t y = 0; y < 128; y+=2) {

        std::size_t texture_i = ((y / 2) * (128 / 8)) + (x / 8);
        std::size_t gfxbuf_i = (y * (128 / 8)) + (x / 8);

        for (std::size_t j = 0; j < 8; j++) {
          std::size_t index = ((texture_i * 8) + j) * 3;
          if (((graphic_ram[gfxbuf_i] >> (7 - j)) & 0x1)) {
            buffer[index] = 0x85;
            buffer[index + 1] = 0xA9;
            buffer[index + 2] = 0xE3;
          } else {
            buffer[index] = 0x0;
            buffer[index + 1] = 0x0;
            buffer[index + 2] = 0xF8;
          }
        }
      }
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void ST7920Device::interrupt(GpioEvent& ev) {
  if (ev.pin_id == clk_pin && ev.event == GpioEvent::FALL && Gpio::pin_map[cs_pin].value){
    incomming_byte = (incomming_byte << 1) | Gpio::pin_map[mosi_pin].value;
    if (++incomming_bit_count == 8) {
      if (incomming_byte_count == 0 && (incomming_byte & 0xF8) != 0xF8) {
        incomming_byte_count++;
      }
      incomming_cmd[incomming_byte_count++] = incomming_byte;
      incomming_byte = incomming_bit_count = 0;
      if (incomming_byte_count == 3) {
        process_command({(incomming_cmd[0] & 0b100) != 0, (incomming_cmd[0] & 0b010) != 0, uint8_t(incomming_cmd[1] | incomming_cmd[2] >> 4)});
        incomming_byte_count = 0;
      }
    }
  } else if (ev.pin_id == cs_pin && ev.event == GpioEvent::RISE) {
    incomming_bit_count = incomming_byte_count = incomming_byte = 0;
  } else if (ev.pin_id == beeper_pin) {
    if (ev.event == GpioEvent::RISE) {
      // play sound
    } else if (ev.event == GpioEvent::FALL) {
      // stop sound
    }
  } else if (ev.pin_id == kill_pin) {
    Gpio::pin_map[kill_pin].value = !key_pressed[KeyName::KILL_BUTTON];
  } else if (ev.pin_id == enc_but_pin) {
    Gpio::pin_map[enc_but_pin].value = !key_pressed[KeyName::ENCODER_BUTTON];
  } else if (ev.pin_id == enc1_pin || ev.pin_id == enc2_pin) {
    uint8_t encoder_state = encoder_position % 4;
    Gpio::pin_map[enc1_pin].value = encoder_table[encoder_state] & 0x01;
    Gpio::pin_map[enc2_pin].value = encoder_table[encoder_state] & 0x02;
  }
}

void ST7920Device::ui_callback(UiWindow* window) {
  if (ImGui::IsWindowFocused()) {
    key_pressed[KeyName::KILL_BUTTON] = ImGui::IsKeyDown(SDL_SCANCODE_K);
    key_pressed[KeyName::ENCODER_BUTTON] = ImGui::IsKeyDown(SDL_SCANCODE_SPACE);
    encoder_position -= ImGui::IsKeyDown(SDL_SCANCODE_UP);
    encoder_position += ImGui::IsKeyDown(SDL_SCANCODE_DOWN);
    if (ImGui::IsWindowHovered()) {
      key_pressed[KeyName::ENCODER_BUTTON] = ImGui::IsMouseClicked(0);
      encoder_position += ImGui::GetIO().MouseWheel > 0 ? 1 : ImGui::GetIO().MouseWheel < 0 ? -1 : 0;
    }
  }
}

#endif

