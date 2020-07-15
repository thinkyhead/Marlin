#pragma once

#define GL_GLEXT_PROTOTYPES

#include "hardware/Clock.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <vector>
#include <array>

constexpr uint32_t steps_per_unit[] = { 80, 80, 80, 500 };

typedef enum t_attrib_id
{
    attrib_position,
    attrib_color
} t_attrib_id;

struct cp_vertex {
  glm::vec4 color;
  glm::vec3 position;
};

class PerspectiveCamera {
public:
  PerspectiveCamera() = default;
  PerspectiveCamera(glm::vec3 position,
                    glm::vec3 focal_point,
                    glm::vec3 world_up,
                    float aspect_ratio,
                    float fov,
                    float clip_near,
                    float clip_far
                    ):position{position},
                    focal_point{focal_point},
                    world_up{world_up},
                    aspect_ratio{aspect_ratio},
                    fov{fov},
                    clip_near{clip_near},
                    clip_far{clip_far},
                    speed{50.0f},
                    front{0.0f, 0.0f, -1.0f} {
    generate();
  }

  void generate() {
    proj = glm::perspective(fov, aspect_ratio, clip_near, clip_far);
    direction = glm::normalize(position - focal_point);
    right = glm::normalize(glm::cross(world_up, direction));
    up = glm::cross(direction, right);
    view = glm::lookAt(position, position - direction, up);

    // glm::extractEulerAngleXYX(-view, rotation.x, rotation.y, rotation.z);
    // rotation.x = glm::degrees(rotation.x);
    // rotation.y = glm::degrees(rotation.y);
    // rotation.z = 0.0f;
    rotation = {-110.599464, 64.200356, 0.000000};
  }

  void update_view() {
    right = glm::normalize(glm::cross(world_up, direction));
    up = glm::cross(direction, right);
    view = glm::lookAt(position, position - direction, up);
  }

  void update_direction() {
    direction.x = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    direction.y = sin(glm::radians(rotation.y));
    direction.z = sin(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
  }

  float* proj_ptr() { return glm::value_ptr(proj); }
  float* view_ptr() { return glm::value_ptr(view); }

  // view
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 focal_point;
  glm::vec3 world_up;
  glm::vec3 right;
  glm::vec3 direction;
  glm::vec3 up;

  // projection
  float aspect_ratio;
  float fov;
  float clip_near;
  float clip_far;
  glm::mat4 view;
  glm::mat4 proj;
  float speed;
  glm::vec3 front;
};

class ShaderProgram {
public:
  static GLuint loadProgram(const char* vertex_string, const char* fragment_string, const char* geometry_string = nullptr) {
    GLuint vertex_shader = 0, fragment_shader = 0, geometry_shader = 0;
    if (vertex_string != nullptr) {
      vertex_shader = loadShader(GL_VERTEX_SHADER, vertex_string);
    }
    if (fragment_string != nullptr) {
      fragment_shader = loadShader(GL_FRAGMENT_SHADER, fragment_string);
    }
    if (geometry_string != nullptr) {
      geometry_shader = loadShader(GL_GEOMETRY_SHADER, geometry_string);
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader( shader_program, vertex_shader );
    glAttachShader( shader_program, fragment_shader );
    if (geometry_shader) glAttachShader( shader_program, geometry_shader );

    glBindAttribLocation(shader_program, attrib_position, "i_position");
    glBindAttribLocation(shader_program, attrib_color, "i_color");
    glLinkProgram(shader_program );
    glUseProgram(shader_program );

    if (vertex_shader) glDeleteShader(vertex_shader);
    if (fragment_shader) glDeleteShader(fragment_shader);
    if (geometry_shader) glDeleteShader(geometry_shader);

    return shader_program;
  }
  static GLuint loadShader(GLuint shader_type, const char* shader_string) {
    GLuint shader_id = glCreateShader(shader_type);;
    int length = strlen(shader_string);
    glShaderSource(shader_id, 1, ( const GLchar ** )&shader_string, &length);
    glCompileShader(shader_id );

    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
      GLint maxLength = 0;
      glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &maxLength);
      std::vector<GLchar> errorLog(maxLength);
      glGetShaderInfoLog(shader_id, maxLength, &maxLength, &errorLog[0]);
      for (auto c : errorLog) fputc(c, stderr);
      glDeleteShader(shader_id);
      return 0;
    }
    return shader_id;
  }
};

class Visualisation {
public:
  Visualisation() : data_thread(&Visualisation::data_update_thread, this) {}
  ~Visualisation() { data_thread_active = false; data_thread.join(); }
  void create(std::size_t width, std::size_t height) {
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    window = SDL_CreateWindow( "Printer Visualisation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    context = SDL_GL_CreateContext( window );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE);

    glClearColor( 0.1, 0.1, 0.1, 0.1 );
    glViewport( 0, 0, width, height );

    const char * geometry_shader = R"SHADERSTR(
      #version 410 core
      layout (lines) in;
      layout (triangle_strip, max_vertices = 14) out;

      in vec4 g_color[];
      out vec4 v_color;

      uniform mat4 u_mvp;
      const float layer_thickness = 0.3; // will be uniforms
      const float layer_width = 0.35;

      void main() {
        vec3 start = gl_in[0].gl_Position.xyz;
        vec3 end = gl_in[1].gl_Position.xyz;
        vec3 direction = end - start;

        vec3 lhs = cross(normalize(direction), vec3(0.0, 0.0, -1.0));
        vec3 up = cross(normalize(direction), vec3(0.0, 1.0, 0.0));

        float half_layer_width = layer_width / 2.0;
        vec3 top_back_left = start - (up * half_layer_width);
        vec3 top_back_right = start + (up * half_layer_width);
        vec3 top_front_left = end - (up * half_layer_width);
        vec3 top_front_right = end + (up * half_layer_width);
        vec3 bottom_back_left = start - (up * half_layer_width) - vec3(0.0, layer_thickness, 0.0);
        vec3 bottom_back_right = start + (up * half_layer_width) - vec3(0.0, layer_thickness, 0.0);
        vec3 bottom_front_left = end - (up * half_layer_width) - vec3(0.0, layer_thickness, 0.0);
        vec3 bottom_front_right = end + (up * half_layer_width) - vec3(0.0, layer_thickness, 0.0);

        v_color = g_color[1];

        gl_Position = u_mvp * vec4( top_front_left, 1.0);//vec4(-1.0, 0.0, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_front_right, 1.0);//vec4( 1.0, 0.0, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( bottom_front_left, 1.0);//vec4(-1.0, -layer_thickness, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( bottom_front_right, 1.0);//vec4( 1.0, -layer_thickness, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( bottom_back_right, 1.0);//;1.0, -layer_thickness, -1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_front_right, 1.0); //1.0, 0.0, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_back_right, 1.0f); //1.0, 0.0, -1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_front_left, 1.0f); //-1.0, 0.0, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_back_left, 1.0f); //-1.0, 0.0, -1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( bottom_front_left, 1.0f); //-1.0, -layer_thickness, 1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( bottom_back_left, 1.0f); //-1.0, -layer_thickness, -1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( bottom_back_right, 1.0f); //1.0, -layer_thickness, -1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_back_left, 1.0f);  //-1.0, 0.0, -1.0, 1.0));
        EmitVertex();
        gl_Position = u_mvp * vec4( top_back_right, 1.0f);  //1.0, 0.0, -1.0, 1.0));
        EmitVertex();
        EndPrimitive();
      };)SHADERSTR";

    const char * path_vertex_shader = R"SHADERSTR(
      #version 410
      in vec3 i_position;
      in vec4 i_color;
      out vec4 g_color;
      void main() {
          g_color = i_color;
          gl_Position = vec4( i_position, 1.0 );
      };)SHADERSTR";

    const char * path_fragment_shader = R"SHADERSTR(
      #version 410
      in vec4 v_color;
      out vec4 o_color;
      void main() {
          if(v_color.a < 0.1) discard;
          o_color = v_color;
      };)SHADERSTR";

    const char * vertex_shader = R"SHADERSTR(
      #version 410
      in vec3 i_position;
      in vec4 i_color;
      out vec4 v_color;
      uniform mat4 u_mvp;
      void main() {
          v_color = i_color;
          gl_Position = u_mvp * vec4( i_position, 1.0 );
      };)SHADERSTR";

    const char * fragment_shader = R"SHADERSTR(
      #version 410
      in vec4 v_color;
      out vec4 o_color;
      void main() {
          o_color = v_color;
      };)SHADERSTR";

    path_program = ShaderProgram::loadProgram(path_vertex_shader, path_fragment_shader, geometry_shader);
    program = ShaderProgram::loadProgram(vertex_shader, fragment_shader);

    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glEnableVertexAttribArray( attrib_position );
    glEnableVertexAttribArray( attrib_color );
    glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof(cp_vertex), 0 );
    glVertexAttribPointer( attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof(cp_vertex), ( void * )(sizeof(cp_vertex::color)) );

    camera = { {50.0f, 200.0f, -200.0f}, {100.0f, 0.0f, -100.0f}, {0.0f, 1.0f, 0.0f}, float(width) / float(height), glm::radians(45.0f), 0.1f, 1000.0f};
    camera.generate();
  }

  void process_event(SDL_Event& e) {
    switch (e.type) {
      case SDL_KEYDOWN: case SDL_KEYUP: {
        if (e.key.windowID == SDL_GetWindowID( window ))
          switch(e.key.keysym.sym) {
            case SDLK_w : {
              input_state[0] = e.type == SDL_KEYDOWN;
              break;
            }
            case SDLK_a : {
              input_state[1] = e.type == SDL_KEYDOWN;
              break;
            }
            case SDLK_s : {
              input_state[2] = e.type == SDL_KEYDOWN;
              break;
            }
            case SDLK_d : {
              input_state[3] = e.type == SDL_KEYDOWN;
              break;
            }
            case SDLK_SPACE : {
              input_state[4] = e.type == SDL_KEYDOWN;
              break;
            }
            case SDLK_LSHIFT : {
              input_state[5] = e.type == SDL_KEYDOWN;
              break;
            }

            case SDLK_f :{
              if( e.type == SDL_KEYUP) {
                follow_mode = !follow_mode;
                if (follow_mode) {
                  camera.position = glm::vec3(effector_pos.x, effector_pos.y + 10.0, effector_pos.z);
                  camera.rotation.y = 89.99999;
                }
              }
              break;
            }
            case SDLK_F1: {
              if (e.type == SDL_KEYUP) {
                SDL_SetRelativeMouseMode((SDL_bool)!mouse_captured);
                mouse_captured = !mouse_captured;
              }
              break;
            }
            case SDLK_F2: {
              if (e.type == SDL_KEYUP) {
                show_full_path = !show_full_path;
              }
              break;
            }
            default:
              break;
          }
        break;
      }
      case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
        if (e.button.button == 1 && e.key.windowID == SDL_GetWindowID( window )) {
          SDL_SetRelativeMouseMode((SDL_bool)!mouse_captured);
          mouse_captured = !mouse_captured;
        }
        break;
      }
      case SDL_MOUSEMOTION: {
        if (mouse_captured) {
          camera.rotation.x += (e.motion.xrel * 0.2f);
          camera.rotation.y += (e.motion.yrel * 0.2f);
          if (camera.rotation.y > 89.0f) camera.rotation.y = 89.0f;
          else if (camera.rotation.y < -89.0f) camera.rotation.y = -89.0f;
        }
        break;
      }
    }
  }

  bool data_thread_active = false;
  bool data_source_set = false;
  int32_t* x_source;
  int32_t* y_source;
  int32_t* z_source;
  int32_t* e_source;
  void set_data_source(int32_t* x, int32_t* y, int32_t* z,  int32_t* e) {
    x_source = x;
    y_source = y;
    z_source = z;
    e_source = e;
    data_source_set = true;
  }
  void data_update_thread() {
    int32_t last_x = 0;
    int32_t last_y = 0;
    int32_t last_z = 0;
    int32_t last_e = 0;
    data_thread_active = true;
    while (data_thread_active) {
      if (data_source_set) {
        int32_t x = *x_source;
        int32_t y = *y_source;
        int32_t z = *z_source;
        int32_t e = *e_source;
        if (x != last_x || y != last_y || z != last_z || e != last_e) {
          set_head_position(glm::vec4{*x_source / (float)steps_per_unit[0],  *z_source / (float)steps_per_unit[2], *y_source / (float)steps_per_unit[1] * -1.0f, *e_source / (float)steps_per_unit[3]});
          last_x = x;
          last_y = y;
          last_z = z;
          last_e = e;
        }
      }
      std::this_thread::yield();
    }
  }

  void update() {
    float delta = (Clock::micros() - last_update) / 1000000.0;
    last_update = Clock::micros();

    if (input_state[0]) {
      camera.position -= camera.speed * camera.direction * delta;
    }
    if (input_state[1]) {
      camera.position += glm::normalize(glm::cross(camera.direction, camera.up)) * camera.speed * delta;
    }
    if (input_state[2]) {
      camera.position += camera.speed * camera.direction * delta;
    }
    if (input_state[3]) {
      camera.position -= glm::normalize(glm::cross(camera.direction, camera.up)) * camera.speed * delta;
    }
    if (input_state[4]) {
      camera.position += camera.world_up * camera.speed * delta;
    }
    if (input_state[5]) {
      camera.position -= camera.world_up * camera.speed * delta;
    }

    camera.update_direction();
    if (follow_mode) {
      camera.position = glm::vec3(effector_pos.x, camera.position.y, effector_pos.z);
    }

    camera.update_view();

    SDL_GL_MakeCurrent(window, context);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glm::mat4 model_tmatrix = glm::translate(glm::mat4(1.0f), glm::vec3(effector_pos.x, effector_pos.y, effector_pos.z));
    glm::mat4 model_smatrix = glm::scale(glm::mat4(1.0f), effector_scale );
    glm::mat4 model_matrix = model_tmatrix * model_smatrix;

    glm::mat4 mvp = camera.proj * camera.view * model_matrix;

    glUseProgram( program );
    glUniformMatrix4fv( glGetUniformLocation( program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    if(!follow_mode) {
      glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), &g_vertex_buffer_data[0], GL_STATIC_DRAW );
      glDrawArrays( GL_TRIANGLES, 0, 18 );
    }

    glm::mat4 bed_matrix = glm::translate(glm::scale(glm::mat4(1.0f), {200.0f, 0.0f, 200.0f}), {0.5f, 0.0, -0.5f});
    mvp = camera.proj * camera.view * bed_matrix;
    glUniformMatrix4fv( glGetUniformLocation( program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
    glDrawArrays( GL_TRIANGLES, 18, 24);

    glm::mat4 print_path_matrix = glm::mat4(1.0f);
    mvp = camera.proj * camera.view * print_path_matrix;
    glUseProgram( path_program );
    glUniformMatrix4fv( glGetUniformLocation( path_program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));

    auto active_path = active_path_block; // a new active path block can be added at any time, so back up the active block ptr;
    std::size_t data_length = active_path->size();
    if (active_path != nullptr && data_length > 1) {
      glBufferData( GL_ARRAY_BUFFER, data_length * sizeof(std::remove_pointer<decltype(active_path)>::type::value_type), &(*active_path)[0], GL_STATIC_DRAW );
      glDrawArrays( GL_LINE_STRIP, 0, data_length);
    }

    if (show_full_path) {
      for (auto& path : full_path) {
        if (&path[0] == &(*active_path)[0]) break;
        std::size_t data_length = path.size();
        glBufferData( GL_ARRAY_BUFFER, data_length * sizeof(std::remove_reference<decltype(path)>::type::value_type), &path[0], GL_STATIC_DRAW );
        glDrawArrays( GL_LINE_STRIP, 0, data_length);
      }
    }

    SDL_GL_SwapWindow( window );

  }

  void destroy() {
    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( window );
    SDL_Quit();
  }

  glm::vec4 last_position = {};
  bool extruding = false;

  void set_head_position(glm::vec4 position) {
    if (position != effector_pos) {
      if (active_path_block != nullptr && active_path_block->size() > 1 && active_path_block->size() < 10000) {

        if (glm::length(glm::vec3(position) - glm::vec3(last_position)) > 0.1f) {
          if(points_are_colinear(position, active_path_block->end()[-2].position, active_path_block->end()[-1].position)) {
            if (extruding == (position.w - last_position.w > 0.0f)) { // extrusion state has not changed to we can just change the current line.
              active_path_block->end()[-1].position = position;
            } else {
              extruding = position.w - last_position.w > 0.0f;
              active_path_block->push_back({{1.0, 0.0, 0.0, extruding ? 1.0 : 0.0}, last_position});
              active_path_block->push_back({{1.0, 0.0, 0.0, extruding ? 1.0 : 0.0}, position});
            }

          } else {
            if (extruding == (position.w - last_position.w > 0.0f)) {
              active_path_block->push_back({{1.0, 0.0, 0.0, extruding ? 1.0 : 0.0}, position});
            } else {
              extruding = position.w - last_position.w > 0.0f;
              active_path_block->push_back({{1.0, 0.0, 0.0, extruding ? 1.0 : 0.0}, last_position});
              active_path_block->push_back({{1.0, 0.0, 0.0, extruding ? 1.0 : 0.0}, position});
            }
          }
          last_position = position;
        }
      } else {
        if (active_path_block == nullptr) {
          full_path.push_back({});
          full_path.back().reserve(10000);
          active_path_block = &full_path.end()[-1];
        } else {
          full_path.push_back({full_path.back().back()});
          full_path.back().reserve(10000);
          active_path_block = &full_path.end()[-1];
        }
        active_path_block->push_back({{1.0, 0.0, 0.0, extruding ? 1.0 : 0.0}, position});

        last_position = position;
      }

      effector_pos = position;
    }
  }

  bool points_are_colinear(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    return glm::length(glm::dot(b - a, c - a) - (glm::length(b - a) * glm::length(c - a))) < 0.0001;
  }

  bool follow_mode = false;
  bool show_full_path = true;
  glm::vec3 follow_offset = {0.0f, 0.0f, 0.0f};
  uint32_t last_update = 0;
  glm::vec4 effector_pos = {};
  glm::vec3 effector_scale = {3.0f ,10.0f, 3.0f};

  std::thread data_thread;
  PerspectiveCamera camera;
  std::vector<cp_vertex>* active_path_block = nullptr;
  std::vector<std::vector<cp_vertex>> full_path;

  SDL_Window * window;
  SDL_GLContext context;
  GLuint program, path_program;
  GLuint vao, vbo;
  bool mouse_captured = false;
  bool input_state[6] = {};

  const std::array<GLfloat, 24 * 7> g_vertex_buffer_data{
      //end effector
      1, 0, 0, 1, 0, 0, 0,
      0, 1, 0, 1, -0.5, 0.5, 0.5,
      0, 0, 1, 1, -0.5, 0.5, -0.5,

      1, 0, 0, 1, 0, 0, 0,
      0, 0, 1, 1, -0.5, 0.5, -0.5,
      0, 1, 0, 1, 0.5, 0.5, -0.5,

      1, 0, 0, 1, 0, 0, 0,
      0, 1, 0, 1, 0.5, 0.5, -0.5,
      0, 0, 1, 1, 0.5, 0.5, 0.5,

      1, 0, 0, 1, 0, 0, 0,
      0, 0, 1, 1, 0.5, 0.5, 0.5,
      0, 1, 0, 1, -0.5, 0.5, 0.5,

      0, 1, 0, 1, 0.5, 0.5, -0.5,
      0, 0, 1, 1, -0.5, 0.5, -0.5,
      0, 1, 0, 1, -0.5, 0.5, 0.5,

      0, 1, 0, 1, 0.5, 0.5, -0.5,
      0, 1, 0, 1, -0.5, 0.5, 0.5,
      0, 0, 1, 1, 0.5, 0.5, 0.5,

      // bed
      0.5, 0.5, 0.5, 1, 0.5, 0, -0.5,
      0.5, 0.5, 0.5, 1, -0.5, 0, -0.5,
      0.5, 0.5, 0.5, 1, -0.5, 0, 0.5,

      0.5, 0.5, 0.5, 1, 0.5, 0, -0.5,
      0.5, 0.5, 0.5, 1, -0.5, 0, 0.5,
      0.5, 0.5, 0.5, 1, 0.5, 0, 0.5,
  };


};