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

  glCreateTextures(GL_TEXTURE_2D, 1, &texture_id);
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

  if (key_pressed[KeyName::ARROW_DOWN]) {
    encoder_rotate_ccw();
    //kernel.delayMicros(50);
    encoder_rotate_ccw();
  }
  if(key_pressed[KeyName::ARROW_UP]) {
    encoder_rotate_cw();
    //kernel.delayMicros(20);
    encoder_rotate_cw();
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
  }
}

void ST7920Device::process_event(SDL_Event& e) {
  switch (e.type) {
    case SDL_KEYDOWN: case SDL_KEYUP: {
      switch(e.key.keysym.sym) {
        case SDLK_k : {
          Gpio::pin_map[kill_pin].value = e.type == SDL_KEYDOWN ? 0 : 1;
          break;
        }
        case SDLK_SPACE: {
          Gpio::pin_map[enc_but_pin].value =  e.type == SDL_KEYDOWN ? 0 : 1;
          break;
        }
        case SDLK_UP: {
          key_pressed[KeyName::ARROW_UP] = e.type == SDL_KEYDOWN;
          break;
        }
        case SDLK_DOWN: {
          key_pressed[KeyName::ARROW_DOWN] = e.type == SDL_KEYDOWN;
          break;
        }
        default:
          break;

      }
      break;
    }
    case SDL_MOUSEWHEEL: {
      if (e.wheel.y > 0 ){
        encoder_rotate_cw();
        //kernel.delayMicros(200);
        encoder_rotate_cw();
      } else {
        encoder_rotate_ccw();
        //kernel.delayMicros(200);
        encoder_rotate_ccw();
      }
      break;
    }
    case SDL_MOUSEBUTTONUP: case SDL_MOUSEBUTTONDOWN: {
      if (e.button.button == SDL_BUTTON_LEFT) {
        Gpio::pin_map[enc_but_pin].value = e.type == SDL_MOUSEBUTTONDOWN ? 0 : 1;
      }
      break;
    }
    case SDL_WINDOWEVENT: {
      if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
        // if (SDL_GetWindowID(window) == e.window.windowID) {
        // }
        close_request = true;
      }
      break;
    }
  }
}

void ST7920Device::encoder_rotate_cw() {
  switch(encoder_position) {
    case 0: {
      Gpio::pin_map[enc1_pin].value = 1;
      Gpio::pin_map[enc2_pin].value = 0;
      encoder_position = 2;
      break;
    }
    case 1: {
      Gpio::pin_map[enc1_pin].value = 0;
      Gpio::pin_map[enc2_pin].value = 0;
      encoder_position = 0;
      break;
    }
    case 2: {
      Gpio::pin_map[enc1_pin].value = 1;
      Gpio::pin_map[enc2_pin].value = 1;
      encoder_position = 3;
      break;
    }
    case 3: {
      Gpio::pin_map[enc1_pin].value = 0;
      Gpio::pin_map[enc2_pin].value = 1;
      encoder_position = 1;
      break;
    }
  }
}

void ST7920Device::encoder_rotate_ccw() {
  switch(encoder_position) {
    case 0: {
      Gpio::pin_map[enc1_pin].value = 0;
      Gpio::pin_map[enc2_pin].value = 1;
      encoder_position = 1;
      break;
    }
    case 1: {
      Gpio::pin_map[enc1_pin].value = 1;
      Gpio::pin_map[enc2_pin].value = 1;
      encoder_position = 3;
      break;
    }
    case 2: {
      Gpio::pin_map[enc1_pin].value = 0;
      Gpio::pin_map[enc2_pin].value = 0;
      encoder_position = 0;
      break;
    }
    case 3: {
      Gpio::pin_map[enc1_pin].value = 1;
      Gpio::pin_map[enc2_pin].value = 0;
      encoder_position = 2;
      break;
    }
  }
}
#endif

