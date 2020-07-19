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
  Visualisation() : data_thread(&Visualisation::data_update_thread, this) {}
  ~Visualisation() { data_thread_active = false; data_thread.join(); }
  void create(std::size_t width, std::size_t height) {
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    window = SDL_CreateWindow( "Printer Visualisation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
    context = SDL_GL_CreateContext( window );

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor( 0.1, 0.1, 0.1, 1.0 );
    glViewport( 0, 0, width, height );

    // todo : Y axis change fix, worked around by not joining
    // todo : very spiky corners after 45 degs, again just worked around by not joining
    const char * geometry_shader =
R"SHADERSTR(
      #version 410 core
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

      const float epsilon = 0.00001;
      bool about_zero(float value) {
        return step(-epsilon, value) * (1.0 - step(epsilon, value)) == 0.0;
      }

      bool collinear_xz(vec3 a, vec3 b, vec3 c){
        return cross(vec3(b.xz, 0) - vec3(a.xz, 0), vec3(c.xz, 0) - vec3(b.xz, 0)) == 0.0;
      }

      bool collinear(vec3 a, vec3 b, vec3 c){
        return cross(b - a, c - b) == 0.0;
      }

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
        if (left == 0.0) return; //panic

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


        // pick on edge cases that ar not taken into account, changle to extrude state, change in z, colliniarity in xy and angle between vectors more than 90 degrees
        if(first_segment || active_color.a != prev_color.a || forward.y > epsilon || collinear_xz(prev, start, end) || dot(start - prev, end - start) < 0.0) {
          start_lhs = left;
          first_segment = true;
        }
        if(last_segment || active_color.a != next_color.a || normalize(next - end).y > epsilon || collinear_xz(start, end, next) || dot(end - start, next - end) < 0.0) {
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
      };
)SHADERSTR";

    const char * path_vertex_shader = R"SHADERSTR(
      #version 410
      in vec3 i_position;
      in vec3 i_normal;
      in vec4 i_color;
      out vec4 g_color;
      out vec3 g_normal;
      void main() {
          g_color = i_color;
          g_normal = i_normal;
          gl_Position = vec4( i_position, 1.0 );
      };)SHADERSTR";

    const char * path_fragment_shader = R"SHADERSTR(
      #version 410
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
      };)SHADERSTR";

    const char * vertex_shader = R"SHADERSTR(
      #version 410
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
      };)SHADERSTR";

    const char * fragment_shader = R"SHADERSTR(
      #version 410
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
      };)SHADERSTR";

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
                follow_mode = follow_mode == 1? 0 : 1;
                if (follow_mode) {
                  camera.position = glm::vec3(effector_pos.x, effector_pos.y + 10.0, effector_pos.z);
                  camera.rotation.y = 89.99999;
                }
              }
              break;
            }
            case SDLK_g :{
              if( e.type == SDL_KEYUP) {
                follow_mode = follow_mode == 2? 0 : 2;
                if (follow_mode) {
                  follow_offset = camera.position;
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
                render_full_path = !render_full_path;
              }
              break;
            }
            case SDLK_F3: {
              if (e.type == SDL_KEYUP) {
                render_path_line = !render_path_line;
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

  using millisec = std::chrono::duration<float, std::milli>;
  void update() {
    auto now = clock.now();
    float delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_update).count();
    last_update = now;

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
    if (follow_mode == 1) {
      camera.position = glm::vec3(effector_pos.x, camera.position.y, effector_pos.z);
    }
    if (follow_mode == 2) {
      camera.position = glm::vec3(camera.position.x, effector_pos.y + follow_offset.y, camera.position.z);
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
    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), &g_vertex_buffer_data[0], GL_STATIC_DRAW );
    if(follow_mode != 1) {
      glDrawArrays( GL_TRIANGLES, 0, 18 );
    }
    glm::mat4 bed_matrix = glm::translate(glm::scale(glm::mat4(1.0f), {200.0f, 0.0f, 200.0f}), {0.5f, 0.0, -0.5f});
    mvp = camera.proj * camera.view * bed_matrix;
    glUniformMatrix4fv( glGetUniformLocation( program, "u_mvp" ), 1, GL_FALSE, glm::value_ptr(mvp));
    glDrawArrays( GL_TRIANGLES, 18, 24);


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

    SDL_GL_SwapWindow( window );

  }

  void destroy() {
    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( window );
    SDL_Quit();
  }

  glm::vec4 last_position = {};
  bool extruding = false;
  const float filiment_diameter = 1.75;

  void set_head_position(glm::vec4 position) {
    if (position != effector_pos) {
      if (active_path_block != nullptr && active_path_block->size() > 1 && active_path_block->size() < 10000) {

        if (glm::length(glm::vec3(position) - glm::vec3(last_position)) > 0.1f) {
          if(points_are_collinear(position, active_path_block->end()[-3].position, active_path_block->end()[-2].position)) {
            if (extruding == (position.w - last_position.w > 0.0f)) { // extrusion state has not changed to we can just change the current line.
              active_path_block->end()[-2].position = position;
              active_path_block->end()[-1].position = position;
            } else {
              extruding = position.w - last_position.w > 0.0f;
              // use the dummy
              active_path_block->end()[-1] = {position, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0, (position.w - last_position.w) * filiment_diameter > 0}};
              // add new dummy
              active_path_block->push_back(active_path_block->back());
            }

          } else {
              extruding = position.w - last_position.w > 0.0f;
              // use the dummy
              active_path_block->end()[-1] ={position, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0, (position.w - last_position.w) * filiment_diameter > 0}};
              // add new dummy
              active_path_block->push_back(active_path_block->back());
          }
          last_position = position;
        }
      } else {
        if (active_path_block == nullptr) {
          full_path.push_back({{position, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0, 0.0}}});
          full_path.back().reserve(10100);
          active_path_block = &full_path.end()[-1];
        } else {
          full_path.push_back({full_path.back().back()});
          full_path.back().reserve(10100);
          active_path_block = &full_path.end()[-1];
        }
        // extra dummy verticies for line strip adjacency
        active_path_block->push_back(active_path_block->back());
        active_path_block->push_back(active_path_block->back());
        active_path_block->push_back(active_path_block->back());
        last_position = position;
      }
      effector_pos = position;
    }
  }

  bool points_are_collinear(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    return glm::length(glm::dot(b - a, c - a) - (glm::length(b - a) * glm::length(c - a))) < 0.0001;
  }

  uint8_t follow_mode = 0;
  bool render_full_path = true;
  bool render_path_line = false;
  glm::vec3 follow_offset = {0.0f, 0.0f, 0.0f};
  std::chrono::high_resolution_clock clock;
  std::chrono::high_resolution_clock::time_point last_update;
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