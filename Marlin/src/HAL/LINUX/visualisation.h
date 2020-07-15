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

#include <vector>
#include <array>


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
  void generate() {
    proj = glm::perspective(fov, aspect_ratio, clip_near, clip_far);
    view = glm::lookAt(position, focal_point, up);
  }

  float* proj_ptr() { return glm::value_ptr(proj); }
  float* view_ptr() { return glm::value_ptr(view); }

  // view
  glm::vec3 position;
  glm::vec3 focal_point;
  glm::vec3 up;

  // projection
  float aspect_ratio;
  float fov;
  float clip_near;
  float clip_far;
  glm::mat4 view;
  glm::mat4 proj;
};

class Visualisation {
public:
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

    vs = glCreateShader( GL_VERTEX_SHADER );
    fs = glCreateShader( GL_FRAGMENT_SHADER );

    const char * vertex_shader =
      "#version 410\n"
      "in vec3 i_position;\n"
      "in vec4 i_color;\n"
      "out vec4 v_color;\n"
      "uniform mat4 u_projection_matrix;\n"
      "uniform mat4 u_view_matrix;\n"
      "uniform mat4 u_model_matrix;\n"
      "void main() {\n"
      "    v_color = i_color;\n"
      "    gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vec4( i_position, 1.0 );\n"
      "}\n";

    const char * fragment_shader =
      "#version 410\n"
      "in vec4 v_color;\n"
      "out vec4 o_color;\n"
      "void main() {\n"
      "    o_color = v_color;\n"
      "}\n";

    int length = strlen( vertex_shader );
    glShaderSource( vs, 1, ( const GLchar ** )&vertex_shader, &length );
    glCompileShader( vs );

    GLint status;
    glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        fprintf( stderr, "vertex shader compilation failed\n" );
        return;// 1;
    }

    length = strlen( fragment_shader );
    glShaderSource( fs, 1, ( const GLchar ** )&fragment_shader, &length );
    glCompileShader( fs );

    glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
    if( status == GL_FALSE )
    {
        fprintf( stderr, "fragment shader compilation failed\n" );
        return;// 1;
    }

    program = glCreateProgram();
    glAttachShader( program, vs );
    glAttachShader( program, fs );

    glBindAttribLocation( program, attrib_position, "i_position" );
    glBindAttribLocation( program, attrib_color, "i_color" );

    glLinkProgram( program );
    glUseProgram( program );

    glEnable( GL_DEPTH_TEST );
    //glEnable( GL_CULL_FACE);
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    glViewport( 0, 0, width, height );

    glGenVertexArrays( 1, &vao );
    glGenBuffers( 1, &vbo );
    glBindVertexArray( vao );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );

    glEnableVertexAttribArray( attrib_position );
    glEnableVertexAttribArray( attrib_color );

    glVertexAttribPointer( attrib_color, 4, GL_FLOAT, GL_FALSE, sizeof( float ) * 7, 0 );
    glVertexAttribPointer( attrib_position, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 7, ( void * )(4 * sizeof(float)) );

    const std::array<GLfloat, 18 * 7> g_vertex_buffer_data{
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

        // bed
        0.5, 0.5, 0.5, 1, 0.5, 0, -0.5,
        0.5, 0.5, 0.5, 1, -0.5, 0, -0.5,
        0.5, 0.5, 0.5, 1, -0.5, 0, 0.5,

        0.5, 0.5, 0.5, 1, 0.5, 0, -0.5,
        0.5, 0.5, 0.5, 1, -0.5, 0, 0.5,
        0.5, 0.5, 0.5, 1, 0.5, 0, 0.5,
    };

    glBufferData( GL_ARRAY_BUFFER, sizeof( g_vertex_buffer_data ), &g_vertex_buffer_data[0], GL_STATIC_DRAW );

    camera = { {50.0f, 200.0f, 200.0f}, {100.0f, 0.0f, -100.0f}, {0.0f, 1.0f, 0.0f}, float(width) / float(height), glm::radians(45.0f), 0.1f, 1000.0f};
    camera.generate();


    glUniformMatrix4fv( glGetUniformLocation( program, "u_projection_matrix" ), 1, GL_FALSE, camera.proj_ptr());
    glUniformMatrix4fv( glGetUniformLocation( program, "u_view_matrix" ), 1, GL_FALSE, camera.view_ptr());
  }

  void process_event(SDL_Event& e) {

  }

  uint32_t last_update = 0;
  glm::vec3 effector_pos = {};
  float effector_rot = 0.0f;
  glm::vec3 effector_scale = {15.0f ,50.0f, 15.0f};

  void update() {
    SDL_GL_MakeCurrent(window, context);

    float delta = (Clock::micros() - last_update) / 1000000.0;
    last_update = Clock::micros();
    effector_rot += (1.0f * delta);

    glm::mat4 model_matrix = glm::rotate(glm::scale(glm::translate(glm::mat4(1.0f), effector_pos), effector_scale ), effector_rot,  {0.0f, 1.0f, 0.0f});

    glUniformMatrix4fv( glGetUniformLocation( program, "u_model_matrix" ), 1, GL_FALSE, glm::value_ptr(model_matrix));
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, 12 );

    glm::mat4 bed_matrix = glm::translate(glm::scale(glm::mat4(1.0f), {200.0f, 0.0f, 200.0f}), {0.5f, 0.0, -0.5f});
    glUniformMatrix4fv( glGetUniformLocation( program, "u_model_matrix" ), 1, GL_FALSE, glm::value_ptr(bed_matrix));
    glDrawArrays( GL_TRIANGLES, 12, 14 );
    SDL_GL_SwapWindow( window );

  }

  void destroy() {
    SDL_GL_DeleteContext( context );
    SDL_DestroyWindow( window );
    SDL_Quit();
  }
  PerspectiveCamera camera = {};

  SDL_Window * window;
  SDL_GLContext context;
  GLuint vs, fs, program;
  GLuint vao, vbo;


};