#ifdef __PLAT_NATIVE_SIM__

#include <mutex>
#include <fstream>
#include <cmath>
#include "Gpio.h"

#include "ST7920Device.h"

#include "serial.h"
extern HalSerial usb_serial;

std::ifstream input_file;

ST7920Device::ST7920Device(pin_type clk, pin_type mosi, pin_type cs,  pin_type beeper, pin_type enc1, pin_type enc2, pin_type enc_but, pin_type kill)
  : clk_pin(clk), mosi_pin(mosi), cs_pin(cs), beeper_pin(beeper), enc1_pin(enc1), enc2_pin(enc2), enc_but_pin(enc_but), kill_pin(kill) {

  Gpio::attach(clk_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(cs_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
  Gpio::attach(beeper_pin, std::bind(&ST7920Device::interrupt, this, std::placeholders::_1));
}

ST7920Device::~ST7920Device() {
  window_destroy();
}

//std::mutex cmd_queue_mutex;

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

  // if (cmd_queue_mutex.try_lock()) {
  //   while (!cmd_in.empty()) {
  //     auto cmd = std::move(cmd_in.front());
  //     process_command(cmd);
  //     cmd_in.pop_front();
  //     cmd_queue_mutex.unlock();
  //   }
  // }
  auto now = clock.now();
  float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_update).count();
  char title_buffer[255];

  double total_seconds = kernel.seconds();
  uint32_t hours = total_seconds / (60 * 60.0);
  double remainder = std::fmod(total_seconds, 60 * 60);
  uint32_t minutes = remainder / 60.0;
  remainder = std::fmod(remainder, 60);


  sprintf(title_buffer, "%02d:%02d:%-66f", hours, minutes, remainder );
  SDL_SetWindowTitle(window, title_buffer);

  if (dirty && delta > 1.0 / 30.0) {
    last_update = now;
    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)graphic_ram, 128, 64, 1, 256 / 8, 0x1, 0x1, 0x1, 0);
    SDL_Surface* optimizedSurface = SDL_ConvertSurface( surf, screenSurface->format, 0 );
    SDL_FreeSurface(surf);
    SDL_Rect rect{0, 0, (int)(128 * scaler), (int)(64 * scaler)};
    if (surf != nullptr) SDL_BlitScaled( optimizedSurface, nullptr, screenSurface, &rect);
    SDL_UpdateWindowSurface( window );
    SDL_FreeSurface(optimizedSurface);
  }

  if (input_file.is_open() && usb_serial.receive_buffer.free()) {
    uint8_t buffer[HalSerial::receive_buffer_size]{};
    auto count = input_file.readsome((char*)buffer, usb_serial.receive_buffer.free());
    usb_serial.receive_buffer.write(buffer, count);
    if(count == 0) input_file.close();
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
        //cmd_queue_mutex.lock();
        //cmd_in.emplace_back(incomming_cmd[0] & 0b100) != 0, (incomming_cmd[0] & 0b010) != 0, uint8_t(incomming_cmd[1] | incomming_cmd[2] >> 4);
        process_command({(incomming_cmd[0] & 0b100) != 0, (incomming_cmd[0] & 0b010) != 0, uint8_t(incomming_cmd[1] | incomming_cmd[2] >> 4)});
        //cmd_queue_mutex.unlock();
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

void ST7920Device::window_create(float scaler) {
  this->scaler = scaler;
  window = SDL_CreateWindow( "ST7920 Emulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 128 * scaler, 64 * scaler, SDL_WINDOW_SHOWN );
  if( window == NULL ) {
    printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
  } else {
    screenSurface = SDL_GetWindowSurface( window );
    SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xAA, 0xAA, 0xAA ) );
    SDL_UpdateWindowSurface( window );
  }
}

void ST7920Device::process_event(SDL_Event& e) {
  switch (e.type) {
    case SDL_KEYDOWN: case SDL_KEYUP: {
      if (e.key.windowID == SDL_GetWindowID( window ))
        switch(e.key.keysym.sym) {
          case SDLK_k : {
            Gpio::set(kill_pin, e.type == SDL_KEYDOWN ? 0 : 1);
            break;
          }
          case SDLK_SPACE: {
            Gpio::set(enc_but_pin, e.type == SDL_KEYDOWN ? 0 : 1);
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
      if (e.wheel.windowID == SDL_GetWindowID( window )) {
        if (e.wheel.y > 0 ){
          encoder_rotate_cw();
          //kernel.delayMicros(200);
          encoder_rotate_cw();
        } else {
          encoder_rotate_ccw();
          //kernel.delayMicros(200);
          encoder_rotate_ccw();
        }
      }
      break;
    }
    case SDL_MOUSEBUTTONUP: case SDL_MOUSEBUTTONDOWN: {
      if (e.button.windowID == SDL_GetWindowID( window ))
        if (e.button.button == SDL_BUTTON_LEFT) {
          Gpio::set(enc_but_pin, e.type == SDL_MOUSEBUTTONDOWN ? 0 : 1);
        }
      break;
    }
    case SDL_DROPFILE: {      // In case if dropped file
        char *dropped_filedir = e.drop.file;
        input_file.open(dropped_filedir);
        SDL_free(dropped_filedir);    // Free dropped_filedir memory
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

void ST7920Device::window_destroy() {
  SDL_DestroyWindow( window );
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

