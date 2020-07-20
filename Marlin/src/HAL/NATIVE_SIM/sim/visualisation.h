#pragma once

#define GL_GLEXT_PROTOTYPES

#include "hardware/LinearAxis.h"

#include <GL/glew.h>
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

using millisec = std::chrono::duration<float, std::milli>;

typedef enum t_attrib_id
{
    attrib_position,
    attrib_normal,
    attrib_color
} t_attrib_id;

struct cp_vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec4 color;
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
    up = glm::normalize(glm::cross(direction, right));
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
  Visualisation();
  ~Visualisation();
  void create(std::size_t width, std::size_t height);
  void process_event(SDL_Event& e);

  LinearAxis x_axis;
  LinearAxis y_axis;
  LinearAxis z_axis;
  LinearAxis extruder0;

  bool data_thread_active = false;

  void gpio_event_handler(GpioEvent& event);
  void update();
  void destroy();

  glm::vec4 last_position = {};
  glm::vec4 last_extrusion_check = {};
  bool extruding = false;
  bool last_extruding  = false;
  const float filiment_diameter = 1.75;
  void set_head_position(glm::vec4 position);
  bool points_are_collinear(glm::vec3 a, glm::vec3 b, glm::vec3 c);

  uint8_t follow_mode = 0;
  bool render_full_path = true;
  bool render_path_line = false;
  glm::vec3 follow_offset = {0.0f, 0.0f, 0.0f};
  std::chrono::high_resolution_clock clock;
  std::chrono::high_resolution_clock::time_point last_update;
  glm::vec4 effector_pos = {};
  glm::vec3 effector_scale = {3.0f ,10.0f, 3.0f};

  PerspectiveCamera camera;
  std::vector<cp_vertex>* active_path_block = nullptr;
  std::vector<std::vector<cp_vertex>> full_path;

  SDL_Window * window;
  SDL_GLContext context;
  GLuint program, path_program;
  GLuint vao, vbo;
  bool mouse_captured = false;
  bool input_state[6] = {};

  const std::array<GLfloat, 24 * 10> g_vertex_buffer_data{
      //end effector
      0, 0, 0, 0.0, 0.0, 0.0, 1, 0, 0, 1,
      -0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,
      -0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 0, 1, 1,

      0, 0, 0, 0.0, 0.0, 0.0, 1, 0, 0, 1,
      -0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 0, 1, 1,
       0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,

      0, 0, 0, 0.0, 0.0, 0.0, 1, 0, 0, 1,
       0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,
       0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 0, 1, 1,

      0, 0, 0, 0.0, 0.0, 0.0, 1, 0, 0, 1,
       0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 0, 1, 1,
      -0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,

       0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,
      -0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 0, 1, 1,
      -0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,

       0.5, 0.5, -0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,
      -0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 1, 0, 1,
       0.5, 0.5, 0.5, 0.0, 0.0, 0.0, 0, 0, 1, 1,

      // bed
      0.5, 0, -0.5, 0.0, 1.0, 0.0, 0.5, 0.5, 0.5, 1,
      -0.5, 0, -0.5, 0.0, 1.0, 0.0, 0.5, 0.5, 0.5, 1,
      -0.5, 0, 0.5, 0.0, 1.0, 0.0, 0.5, 0.5, 0.5, 1,

      0.5, 0, -0.5, 0.0, 1.0, 0.0, 0.5, 0.5, 0.5, 1,
      -0.5, 0, 0.5, 0.0, 1.0, 0.0, 0.5, 0.5, 0.5, 1,
      0.5, 0, 0.5, 0.0, 1.0, 0.0, 0.5, 0.5, 0.5, 1,
  };


};