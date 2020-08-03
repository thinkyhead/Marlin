#ifdef __PLAT_NATIVE_SIM__

#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>

#include "user_interface.h"
#include "application.h"

Application::Application() {
  sim.vis.create();

  user_interface.addElement<SerialMonitor>("Serial Monitor");
  user_interface.addElement<TextureWindow>("Controller Display", sim.display.texture_id, 128.0 / 64.0, std::bind(&ST7920Device::ui_callback, &sim.display, std::placeholders::_1));
  user_interface.addElement<StatusWindow>("Status", &clear_color, std::bind(&Visualisation::ui_info_callback, &sim.vis, std::placeholders::_1));
  user_interface.addElement<Viewport>("Viewport", std::bind(&Visualisation::ui_viewport_callback, &sim.vis, std::placeholders::_1));
  //user_interface.addElement<GraphWindow>("graphs", sim.display.texture_id, 128.0 / 64.0, std::bind(&Simulation::ui_callback, &sim, std::placeholders::_1));
}

Application::~Application() {

}

void Application::update() {
  auto viewport = std::dynamic_pointer_cast<Viewport>(user_interface.ui_elements["Viewport"]); // can get ui elemts by name
  auto display = std::dynamic_pointer_cast<TextureWindow>(user_interface.ui_elements["Controller Display"]); // can get ui elemts by name

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT) {
      active = false;
    }
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID((SDL_Window*)window.getHandle())) {
      active = false;
    }

    if (event.type == SDL_DROPFILE) {
      char *dropped_filedir = event.drop.file;
      input_file.open(dropped_filedir);
      SDL_free(dropped_filedir);    // Free dropped_filedir memory
    }

  }

  // File read into serial port
  if (input_file.is_open() && usb_serial.receive_buffer.free()) {
    uint8_t buffer[HalSerial::receive_buffer_size]{};
    auto count = input_file.readsome((char*)buffer, usb_serial.receive_buffer.free());
    usb_serial.receive_buffer.write(buffer, count);
    if(count == 0) input_file.close();
  }

  sim.update();
  user_interface.show();
}

void Application::render() {
  sim.vis.framebuffer->bind();
  glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  sim.vis.update(); //update and render
  sim.vis.framebuffer->render(); // render and unbind framebuffer

  user_interface.render();
  window.swap_buffers();
}

#endif //__PLAT_NATIVE_SIM__