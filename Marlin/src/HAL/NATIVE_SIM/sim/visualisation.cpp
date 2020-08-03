#ifdef __PLAT_NATIVE_SIM__

#include "visualisation.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <vector>
#include <array>
#include <imgui_internal.h>
#include <implot.h>

#include "hardware/LinearAxis.h"
#include "src/inc/MarlinConfig.h"



constexpr uint32_t steps_per_unit[] = DEFAULT_AXIS_STEPS_PER_UNIT;

Visualisation::Visualisation() :
  x_axis(X_ENABLE_PIN, X_DIR_PIN, X_STEP_PIN, X_MIN_PIN, X_MAX_PIN, INVERT_X_DIR),
  y_axis(Y_ENABLE_PIN, Y_DIR_PIN, Y_STEP_PIN, Y_MIN_PIN, Y_MAX_PIN, INVERT_Y_DIR),
  z_axis(Z_ENABLE_PIN, Z_DIR_PIN, Z_STEP_PIN, Z_MIN_PIN, Z_MAX_PIN, INVERT_Z_DIR),
  extruder0(E0_ENABLE_PIN, E0_DIR_PIN, E0_STEP_PIN, P_NC, P_NC, INVERT_E0_DIR) {
    Gpio::attach(x_axis.step_pin, [this](GpioEvent &event){ this->gpio_event_handler(event); }); //todo this works too
    Gpio::attach(x_axis.min_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(x_axis.max_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(y_axis.step_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(y_axis.min_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(y_axis.max_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(z_axis.step_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(z_axis.min_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(z_axis.max_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(extruder0.step_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(extruder0.min_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
    Gpio::attach(extruder0.max_pin, std::bind(&Visualisation::gpio_event_handler, this, std::placeholders::_1));
  }

Visualisation::~Visualisation() {
  destroy();
}

void Visualisation::create() {
  // todo : Y axis change fix, worked around by not joining
  // todo : very spiky corners after 45 degs, again just worked around by not joining
  const char * geometry_shader =
R"SHADERSTR(
    #version 330 core
    layout (lines_adjacency) in;
    layout (triangle_strip, max_vertices = 28) out;

    in vec3 g_normal[];
    in vec4 g_color[];
    out vec4 v_color;
    out vec3 v_normal;
    out vec4 v_position;

    uniform mat4 u_mvp;
    const float layer_thickness = 0.3;
    const float layer_width = 0.4;

    vec4 mvp_vertices[9];
    vec4 vertices[9];
    void emit(const int a, const int b, const int c, const int d) {
      gl_Position = mvp_vertices[a]; v_position = vertices[a]; EmitVertex();
      gl_Position = mvp_vertices[b]; v_position = vertices[b]; EmitVertex();
      gl_Position = mvp_vertices[c]; v_position = vertices[c]; EmitVertex();
      gl_Position = mvp_vertices[d]; v_position = vertices[d]; EmitVertex();
      EndPrimitive();
    }

    const float epsilon = 0.000001;

    void main() {
      vec3 prev = gl_in[0].gl_Position.xyz;
      vec3 start = gl_in[1].gl_Position.xyz;
      vec3 end = gl_in[2].gl_Position.xyz;
      vec3 next = gl_in[3].gl_Position.xyz;

      vec4 prev_color = g_color[1];
      vec4 active_color = g_color[2];
      vec4 next_color = g_color[3];

      vec3 forward = normalize(end - start);
      vec3 left = normalize(cross(forward, g_normal[2])); // what if formward is world_up? zero vector
      if (left == vec3(0.0)) return; //panic

      vec3 up = normalize(cross(forward, left));
      up *= sign(up); // make sure up is positive

      bool first_segment = length(start - prev) < epsilon;
      bool last_segment = length(end - next) < epsilon;
      vec3 a = normalize(start - prev);
      vec3 b = normalize(start - end);
      vec3 c = (a + b) * 0.5;
      vec3 start_lhs = normalize(c) * sign(dot(c, left));
      a = normalize(end - start);
      b = normalize(end - next);
      c = (a + b) * 0.5;
      vec3 end_lhs = normalize(c) * sign(dot(c, left));

      vec2 xz_dir_a = normalize(start.xz - prev.xz);
      vec2 xz_dir_b = normalize(end.xz - start.xz);
      vec2 xz_dir_c = normalize(next.xz - end.xz);


      // remove edge cases that will break teh following algorithm, angle more than 90 degrees or 0 (points collinear), also break on extrude state change (color)
      if(first_segment || active_color.a != prev_color.a || normalize(next - prev).y != 0.0 ||  dot(normalize(start.xz - prev.xz), normalize(end.xz - start.xz)) < epsilon || dot(normalize(start.xz - prev.xz), normalize(end.xz - start.xz)) > 1.0 - epsilon) {
        start_lhs = left;
        first_segment = true;
      }
      if(last_segment || active_color.a != next_color.a || normalize(next - prev).y != 0.0 || dot(normalize(end.xz - start.xz), normalize(next.xz - end.xz)) < epsilon || dot(normalize(end.xz - start.xz), normalize(next.xz - end.xz)) > 1.0 - epsilon) {
        end_lhs = left;
        last_segment = true;
      }

      float start_join_scale = dot(start_lhs, left);
      float end_join_scale = dot(end_lhs, left);
      start_lhs *= layer_width * 0.5;
      end_lhs *= layer_width * 0.5;

      float half_layer_width = layer_width / 2.0;
      vertices[0] = vec4(start - start_lhs / start_join_scale, 1.0); // top_back_left
      vertices[1] = vec4(start + start_lhs / start_join_scale, 1.0); // top_back_right
      vertices[2] = vec4(end   - end_lhs / end_join_scale, 1.0);   // top_front_left
      vertices[3] = vec4(end   + end_lhs / end_join_scale, 1.0);   // top_front_right
      vertices[4] = vec4(start - start_lhs / start_join_scale - (up * layer_thickness), 1.0); // bottom_back_left
      vertices[5] = vec4(start + start_lhs / start_join_scale - (up * layer_thickness), 1.0); // bottom_back_right
      vertices[6] = vec4(end   - end_lhs / end_join_scale - (up * layer_thickness), 1.0);   // bottom_front_left
      vertices[7] = vec4(end   + end_lhs / end_join_scale - (up * layer_thickness), 1.0);   // bottom_front_right

      mvp_vertices[0] = u_mvp * vertices[0];
      mvp_vertices[1] = u_mvp * vertices[1];
      mvp_vertices[2] = u_mvp * vertices[2];
      mvp_vertices[3] = u_mvp * vertices[3];
      mvp_vertices[4] = u_mvp * vertices[4];
      mvp_vertices[5] = u_mvp * vertices[5];
      mvp_vertices[6] = u_mvp * vertices[6];
      mvp_vertices[7] = u_mvp * vertices[7];

      vertices[8] = vec4(start - (left * half_layer_width) + (up * 1.0), 1.0);
      mvp_vertices[8] = u_mvp * vertices[8];

      v_color = active_color;
      v_normal = forward;
      if (last_segment) emit(2, 3, 6, 7); // thise should be rounded ends of path diamter, not single po
      v_normal = -forward;
      if (first_segment) emit(1, 0, 5, 4);
      v_normal = -left;
      emit(3, 1, 7, 5);
      v_normal = left;
      emit(0, 2, 4, 6);
      v_normal = up;
      emit(0, 1, 2, 3);
      v_normal = -up;
      emit(5, 4, 7, 6);

      //emit(0, 1, 8, 0); //show up normal
    }
)SHADERSTR";

  const char * path_vertex_shader = R"SHADERSTR(
    #version 330
    in vec3 i_position;
    in vec3 i_normal;
    in vec4 i_color;
    out vec4 g_color;
    out vec3 g_normal;
    void main() {
        g_color = i_color;
        g_normal = i_normal;
        gl_Position = vec4( i_position, 1.0 );
    })SHADERSTR";

  const char * path_fragment_shader = R"SHADERSTR(
    #version 330
    in vec4 v_color;
    out vec4 o_color;
    in vec3 v_normal;
    in vec4 v_position;

    uniform vec3 u_view_position;
    void main() {
        if(v_color.a < 0.1) discard;

        float ambient_level = 0.1;
        vec3 ambient_color = vec3(1.0, 0.86, 0.66);
        vec3 ambient = ambient_color * ambient_level;

        vec3 light_position = vec3(0,300,0);
        vec3 norm = normalize(v_normal);
        vec3 light_direction = light_position - v_position.xyz;
        float d = length(light_direction);
        float attenuation = 1.0 / ( 1.0 + 0.005 * d); // simplication of 1.0/(1.0 + c1*d + c2*d^2)
        light_direction = normalize(light_direction);
        vec3 diffuse_color = ambient_color;
        float diff = max(dot(norm, light_direction), 0.0);
        vec3 diffuse = diff * diffuse_color;

        float specular_strength = 0.5;
        vec3 view_direction = normalize(u_view_position - v_position.xyz);
        vec3 reflect_direction = reflect(-light_direction, norm);

        float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 32);
        vec3 specular = specular_strength * spec * diffuse_color;

        if(v_color.a < 0.1) {
          o_color = vec4(vec3(0.0, 0.0, 1.0) * (ambient + ((diffuse + specular) * attenuation)), v_color.a);
        } else {
          o_color = vec4(v_color.rgb * (ambient + ((diffuse + specular) * attenuation)), v_color.a);
        }
    })SHADERSTR";

  const char * vertex_shader = R"SHADERSTR(
    #version 330
    in vec3 i_position;
    in vec3 i_normal;
    in vec4 i_color;
    out vec4 v_color;
    out vec3 v_normal;
    out vec3 v_position;
    uniform mat4 u_mvp;
    void main() {
        v_color = i_color;
        v_normal = i_normal;
        v_position = i_position;
        gl_Position = u_mvp * vec4( i_position, 1.0 );
    })SHADERSTR";

  const char * fragment_shader = R"SHADERSTR(
    #version 330
    in vec4 v_color;
    in vec3 v_normal;
    in vec3 v_position;
    out vec4 o_color;
    void main() {
        if(v_color.a < 0.1) {
          //discard;
          o_color = vec4(0.0, 0.0, 1.0, 1.0);
        } else {
          o_color = v_color;
        }
    })SHADERSTR";

  path_program = ShaderProgram::loadProgram(path_vertex_shader, path_fragment_shader, geometry_shader);
  program = ShaderProgram::loadProgram(vertex_shader, fragment_shader);

  glGenVertexArrays( 1, &vao );
  glGenBuffers( 1, &vbo );
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glEnableVertexAttribArray( attrib_position );
  glEnableVertexAttribArray( attrib_normal );
  glEnableVertexAttribArray( attrib_color );
  glVertexAttribPointer( attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(cp_vertex), 0 );
  glVertexAttribPointer( attrib_normal, 3, GL_FLOAT, GL_FALSE, sizeof(cp_vertex), ( void * )(sizeof(cp_vertex::position)) );
  glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(cp_vertex), ( void * )(sizeof(cp_vertex::position) + sizeof(cp_vertex::normal)) );

  framebuffer = new opengl_util::MsaaFrameBuffer();
  if (!((opengl_util::MsaaFrameBuffer*)framebuffer)->create(100, 100, 4)) {
    fprintf(stderr, "Failed to initialise MSAA Framebuffer falling back to TextureFramebuffer\n");
    delete framebuffer;
    framebuffer = new opengl_util::TextureFrameBuffer();
    if (!((opengl_util::TextureFrameBuffer*)framebuffer)->create(100,100)) {
      fprintf(stderr, "Unable to initialise a Framebuffer\n");
    }
  }

  camera = { {37.0f, 121.0f, 129.0f}, {-192.0f, -25.0, 0.0f}, {0.0f, 1.0f, 0.0f}, float(100) / float(100), glm::radians(45.0f), 0.1f, 1000.0f};
  camera.generate();
}

void Visualisation::process_event(SDL_Event& e) {

}

void Visualisation::gpio_event_handler(GpioEvent& event) {
  x_axis.interrupt(event);
  y_axis.interrupt(event);
  z_axis.interrupt(event);
  extruder0.interrupt(event);
  set_head_position(glm::vec4{x_axis.position / (float)steps_per_unit[0], z_axis.position / (float)steps_per_unit[2], y_axis.position / (float)steps_per_unit[1] * -1.0f, extruder0.position  / (float)steps_per_unit[3]});
}

using millisec = std::chrono::duration<float, std::milli>;
void Visualisation::update() {
  // auto now = clock.now();
  // float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_update).count();
  // last_update = now;

  if (follow_mode == 1) {
    camera.position = glm::vec3(effector_pos.x, camera.position.y, effector_pos.z);
  }
  if (follow_mode == 2) {
    camera.position = glm::vec3(camera.position.x, effector_pos.y + follow_offset.y, camera.position.z);
  }
  camera.update_view();

  glm::mat4 model_tmatrix = glm::translate(glm::mat4(1.0f), glm::vec3(effector_pos.x, effector_pos.y, effector_pos.z));
  glm::mat4 model_smatrix = glm::scale(glm::mat4(1.0f), effector_scale );
  glm::mat4 model_matrix = model_tmatrix * model_smatrix;

  glm::mat4 mvp = camera.proj * camera.view * model_matrix;

  glUseProgram( program );
  glUniformMatrix4fv( glGetUniformLocation( program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), &g_vertex_buffer_data[0], GL_STATIC_DRAW );
  if(follow_mode != 1) {
    glDrawArrays( GL_TRIANGLES, 0, 18 );
  }
  glm::mat4 bed_matrix = glm::translate(glm::scale(glm::mat4(1.0f), {200.0f, 0.0f, 200.0f}), {0.5f, 0.0, -0.5f});
  mvp = camera.proj * camera.view * bed_matrix;
  glUniformMatrix4fv( glGetUniformLocation( program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
  glDrawArrays( GL_TRIANGLES, 18, 24);

  if (active_path_block != nullptr) {
    glm::mat4 print_path_matrix = glm::mat4(1.0f);
    mvp = camera.proj * camera.view * print_path_matrix;
    glUniformMatrix4fv( glGetUniformLocation( program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
    auto active_path = active_path_block; // a new active path block can be added at any time, so back up the active block ptr;
    std::size_t data_length = active_path->size();

    if (render_path_line) {
      if (active_path != nullptr && data_length > 1) {
        glBufferData( GL_ARRAY_BUFFER, data_length * sizeof(std::remove_pointer<decltype(active_path)>::type::value_type), &(*active_path)[0], GL_STATIC_DRAW );
        glDrawArrays( GL_LINE_STRIP_ADJACENCY, 0, data_length);
      }

      if (render_full_path) {
        for (auto& path : full_path) {
          if (&path[0] == &(*active_path)[0]) break;
          // these are no longer dynamic buffers and could have the geometry baked rather than continue using the geometery shader
          std::size_t data_length = path.size();
          glBufferData( GL_ARRAY_BUFFER, data_length * sizeof(std::remove_reference<decltype(path)>::type::value_type), &path[0], GL_STATIC_DRAW );
          glDrawArrays( GL_LINE_STRIP_ADJACENCY, 0, data_length);
        }
      }
    }

    glUseProgram( path_program );
    glUniformMatrix4fv( glGetUniformLocation( path_program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3fv( glGetUniformLocation( path_program, "u_view_position" ), 1, glm::value_ptr(camera.position));

    if (active_path != nullptr && data_length > 1) {
      glBufferData( GL_ARRAY_BUFFER, data_length * sizeof(std::remove_pointer<decltype(active_path)>::type::value_type), &(*active_path)[0], GL_STATIC_DRAW );
      glDrawArrays( GL_LINE_STRIP_ADJACENCY, 0, data_length);
    }

    if (render_full_path) {
      for (auto& path : full_path) {
        if (&path[0] == &(*active_path)[0]) break;
        // these are no longer dynamic buffers and could have the geometry baked rather than continue using the geometery shader
        std::size_t data_length = path.size();
        glBufferData( GL_ARRAY_BUFFER, data_length * sizeof(std::remove_reference<decltype(path)>::type::value_type), &path[0], GL_STATIC_DRAW );
        glDrawArrays( GL_LINE_STRIP_ADJACENCY, 0, data_length);
      }
    }
  }

}

void Visualisation::destroy() {
  if(framebuffer != nullptr) {
    framebuffer->release();
    delete framebuffer;
  }
}

void Visualisation::set_head_position(glm::vec4 position) {
  if (position != effector_pos) {

    if (glm::length(glm::vec3(position) - glm::vec3(last_extrusion_check)) > 0.5f) { // smooths out extrusion over a minimum length to fill in gaps todo: implement an simulation to do this better
      extruding = position.w - last_extrusion_check.w > 0.0f;
      last_extrusion_check = position;
    }

    if (active_path_block != nullptr && active_path_block->size() > 1 && active_path_block->size() < 10000) {

      if (glm::length(glm::vec3(position) - glm::vec3(last_position)) > 0.05f) { // smooth out the path so the model renders with less geometry, rendering each individual step hurts the fps
        if(points_are_collinear(position, active_path_block->end()[-3].position, active_path_block->end()[-2].position) && extruding == last_extruding) {
          // collinear and extrusion state has not changed to we can just change the current point.
          active_path_block->end()[-2].position = position;
          active_path_block->end()[-1].position = position;
        } else { // new point is not collinear with current path add new point
          active_path_block->end()[-1] ={position, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0, extruding}};
          active_path_block->push_back(active_path_block->back());
        }
        last_position = position;
        last_extruding = extruding;
      }

    } else { // need to change geometry buffer
      if (active_path_block == nullptr) {
        full_path.push_back({{position, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0, 0.0}}});
        full_path.back().reserve(10100);
        active_path_block = &full_path.end()[-1];
        active_path_block->push_back(active_path_block->back());
        last_extrusion_check = position;
      } else {
        full_path.push_back({full_path.back().back()});
        full_path.back().reserve(10100);
        active_path_block = &full_path.end()[-1];
        active_path_block->push_back({position, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0, extruding}});
        active_path_block->push_back(active_path_block->back());
      }
      // extra dummy verticies for line strip adjacency
      active_path_block->push_back(active_path_block->back());
      active_path_block->push_back(active_path_block->back());
      last_position = position;
    }
    effector_pos = position;
  }
}

bool Visualisation::points_are_collinear(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  return glm::length(glm::dot(b - a, c - a) - (glm::length(b - a) * glm::length(c - a))) < 0.0002; // could be increased to further reduce rendered geometry
}


void Visualisation::ui_viewport_callback(UiWindow* window) {
  auto now = clock.now();
  float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now- last_update).count();
  last_update = now;

  Viewport& viewport = *((Viewport*)window);

  if (viewport.dirty) {
      framebuffer->update(viewport.viewport_size.x, viewport.viewport_size.y);
      viewport.texture_id = framebuffer->texture_id();
      camera.update_aspect_ratio(viewport.viewport_size.x / viewport.viewport_size.y);
  }

  if (viewport.focused) {
    if (ImGui::IsKeyDown(SDL_SCANCODE_W)) {
      camera.position += camera.speed * camera.direction * delta;
    }
    if (ImGui::IsKeyDown(SDL_SCANCODE_S)) {
      camera.position -= camera.speed * camera.direction * delta;
    }
    if (ImGui::IsKeyDown(SDL_SCANCODE_A)) {
      camera.position -= glm::normalize(glm::cross(camera.direction, camera.up)) * camera.speed * delta;
    }
    if (ImGui::IsKeyDown(SDL_SCANCODE_D)) {
      camera.position += glm::normalize(glm::cross(camera.direction, camera.up)) * camera.speed * delta;
    }
    if (ImGui::IsKeyDown(SDL_SCANCODE_SPACE)) {
      camera.position += camera.world_up * camera.speed * delta;
    }
    if (ImGui::IsKeyDown(SDL_SCANCODE_LSHIFT)) {
      camera.position -= camera.world_up * camera.speed * delta;
    }
    if (ImGui::IsKeyPressed(SDL_SCANCODE_F)) {
      follow_mode = follow_mode == 1? 0 : 1;
      if (follow_mode) {
        camera.position = glm::vec3(effector_pos.x, effector_pos.y + 10.0, effector_pos.z);
        camera.rotation.y = -89.99999;
      }
    }
    if (ImGui::IsKeyPressed(SDL_SCANCODE_G)) {
      follow_mode = follow_mode == 2? 0 : 2;
      if (follow_mode) {
        follow_offset = camera.position - glm::vec3(effector_pos);
      }
    }
    if (ImGui::IsKeyPressed(SDL_SCANCODE_F1)) {
      render_full_path = !render_full_path;
    }
    if (ImGui::IsKeyPressed(SDL_SCANCODE_F2)) {
      render_path_line = !render_path_line;
    }

    if (ImGui::GetIO().MouseWheel != 0 && viewport.hovered) {
      camera.position += camera.speed * camera.direction * delta * ImGui::GetIO().MouseWheel;
    }
  }

  bool last_mouse_captured = mouse_captured;
  if (ImGui::IsMouseDown(0) && viewport.hovered) {
    mouse_captured = true;
  } else if (!ImGui::IsMouseDown(0)) {
    mouse_captured = false;
  }

  if (mouse_captured && !last_mouse_captured) {
    ImVec2 mouse_pos = ImGui::GetMousePos();
    mouse_lock_pos = {mouse_pos.x, mouse_pos.y};
    SDL_SetWindowGrab(SDL_GL_GetCurrentWindow(), SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
  } else if (!mouse_captured && last_mouse_captured) {
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_SetWindowGrab(SDL_GL_GetCurrentWindow(), SDL_FALSE);
    SDL_WarpMouseInWindow(SDL_GL_GetCurrentWindow(), mouse_lock_pos.x, mouse_lock_pos.y);
  } else if (mouse_captured) {
    camera.rotation.x -= ImGui::GetIO().MouseDelta.x * 0.2;
    camera.rotation.y -= ImGui::GetIO().MouseDelta.y * 0.2;
    if (camera.rotation.y > 89.0f) camera.rotation.y = 89.0f;
    else if (camera.rotation.y < -89.0f) camera.rotation.y = -89.0f;
  }
};


// Test graph

struct ScrollingData {
    int MaxSize;
    int Offset;
    ImVector<ImPlotPoint> Data;
    ScrollingData() {
        MaxSize = 2000;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImPlotPoint(x,y));
        else {
            Data[Offset] = ImPlotPoint(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

void Visualisation::ui_info_callback(UiWindow*) {
  if (kernel.timing_mode == Kernel::TimingMode::ISRSTEP) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  ui_realtime_scale = kernel.realtime_scale.load();
  ImGui::SliderFloat("sim speed 100", &ui_realtime_scale, 0.0f, 100.0f);
  ImGui::SliderFloat("sim speed 10", &ui_realtime_scale, 0.0f, 10.0f);
  ImGui::SliderFloat("sim speed 1", &ui_realtime_scale, 0.0f, 1.0f);
  ImGui::SliderFloat("sim speed 0.1", &ui_realtime_scale, 0.0f, 0.1f);
  kernel.realtime_scale.store(ui_realtime_scale);
  if (kernel.timing_mode == Kernel::TimingMode::ISRSTEP) {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
  }

  uint64_t time_source = kernel.ticksToNanos(kernel.getTicks());
  uint64_t hours = (time_source / (Kernel::ONE_BILLION * 60 * 60)) ;
  uint64_t remainder = (time_source % (Kernel::ONE_BILLION * 60 * 60));
  uint64_t mins = (remainder / (Kernel::ONE_BILLION * 60));
  remainder = (remainder % (Kernel::ONE_BILLION * 60));
  uint64_t seconds = remainder / (Kernel::ONE_BILLION);
  remainder = remainder % (Kernel::ONE_BILLION);
  ImGui::Text("%02ld:%02ld:%02ld.%ld", hours, mins, seconds, remainder);
  ImGui::Text("ISR timing error: %ldns", kernel.isr_timing_error.load());


  // lock the toggle button until the mode has been changed as it may be blocked
  bool disabled_toggle = kernel.timing_mode != kernel.timing_mode_toggle;
  if (disabled_toggle) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
  }
  ImGui::Checkbox("Disable Realtime Mode ", (bool*)&kernel.timing_mode_toggle);
  if (disabled_toggle) {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
  }


  static bool paused = false;
  static ScrollingData sdata1, sdata2;
  //static RollingData   rdata1, rdata2;
  static float t = 0.0;
  if (!paused) {
      t += ImGui::GetIO().DeltaTime;
      sdata1.AddPoint(t, kernel.isr_timing_error.load());
      //rdata1.AddPoint(t, mouse.x * 0.0005f);
      //sdata2.AddPoint(t, kernel.realtime_scale.load());
      //rdata2.AddPoint(t, mouse.y * 0.0005f);
  }
  static float history = 10.0f;
  ImGui::SliderFloat("History",&history,1,30,"%.1f s");
  // rdata1.Span = history;
  // rdata2.Span = history;
  ImPlot::SetNextPlotLimitsX(t - history, t, paused ? ImGuiCond_Once : ImGuiCond_Always);
  static int rt_axis = ImPlotAxisFlags_Default | ImPlotAxisFlags_LockMin;
  if (ImPlot::BeginPlot("##Scrolling", NULL, NULL, ImVec2(-1,150), ImPlotFlags_Default, rt_axis & ~ImPlotAxisFlags_TickLabels, rt_axis)) {
      ImPlot::PlotLine("ISR Timing Error (ns)", &sdata1.Data[0].x, &sdata1.Data[0].y, sdata1.Data.size(), sdata1.Offset, sizeof(ImPlotPoint));
      //ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
      //ImPlot::PlotShaded("Data 2", &sdata2.Data[0].x, &sdata2.Data[0].y, sdata2.Data.size(), 0, sdata2.Offset, sizeof(ImPlotPoint));
      //ImPlot::PopStyleVar();
      ImPlot::EndPlot();
  }
  ImGui::Text("Double left click to autoscale, right click contect menu");

}

#endif
