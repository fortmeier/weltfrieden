// header missing

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <getopt.h>



#include "config.h"
#include "gl_env.h"
#include "dbg.h"
#include "layers.h"
#include "shader.h"
#include "text.h"

extern int server_init(void);

static double start_time;
extern double now;
extern float res[2];

extern GLuint vao_texcoord;
extern GLuint vbo;
extern GLuint vao;
extern float cursor[2];

extern int scribble;

int last_scribble = 0;

extern int shader_lvl;

int cache;

float points[] = {
  1.0, 1.0, 0.0, 1.0, 0.0,
  -1.0, 1.0, 0.0, 0.0, 0.0,
  1.0, -1.0, 0.0, 1.0, 1.0,
  -1.0, -1.0, 0.0, 0.0, 1.0
};

void init(void) {
  server_init();
}

void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
  if (scribble == 1) {
    cursor[0] = (float)xpos;
    cursor[1] = (float) (res[1] - ypos);
  }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    scribble = (action == GLFW_PRESS) ? 1 : 0;

    if (last_scribble != scribble) {
      layers_redraw_scribble();
    }
  }
}

void reshape( GLFWwindow* window, int width, int height )
{
  glViewport(0, 0, width, height);
  /* mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1); */
}

void render(GLFWwindow* win) {
  double n = glfwGetTime();
  now = start_time + n;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  layers_apply();
  layers_cleanup();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glfwSwapBuffers(win);
}


int main(int argc, char **argv) {
  int c;

  cache = 0;

  while (1) {
    static struct option long_options[] =
      {
        {"cache", no_argument, &cache, 1},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0,0,0,0}
      };

    int option_index = 0;

    c = getopt_long(argc, argv, "w:h:", long_options, &option_index);

    if (c == -1) {
      break;
    }

    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0) break;
    case 'w':
      res[0] = atoi(optarg);
      break;
    case 'h':
      res[1] = atoi(optarg);
      break;
    case 'v':
      log_info("Version: 0.0.0, not yet implemented");
      break;
    default:
      return 1;
    }
  }



  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    log_err("Error: cannot setup glfw.\n");
    return 1;
  }

  /* glfwWindowHint(GLFW_SAMPLES, 4); */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #ifdef EGL_RPI2
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  #endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_FLOATING, GL_TRUE);
  GLFWwindow* win = NULL;

  shader_lvl = 0;

  win = glfwCreateWindow(1, 1, "GLFW", NULL, NULL);
  if(!win) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    log_info("Falling back to OpenGL 2.1\n");
    shader_lvl = 1;
    win = glfwCreateWindow(1, 1, "GLFW", NULL, NULL);
    if (!win) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
  }
  else {
    shader_lvl = 3;
  }

  /* mat4_set_identity( &projection ); */
  /* mat4_set_identity( &model ); */
  /* mat4_set_identity( &view ); */

  glfwMakeContextCurrent(win);
  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString (GL_VERSION); // version as a string
  log_info("Renderer: %s\n", renderer);
  log_info("OpenGL version supported %s\n", version);

  log_info("Shading Language Level: %dxx", shader_lvl);

  /* glViewport(0, 0, w, h); */
  glfwSwapInterval(1);
  glfwSetFramebufferSizeCallback( win, reshape );
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  /* glDisable(GL_BLEND); */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glGenBuffers (1, &vbo);
  glBindBuffer (GL_ARRAY_BUFFER, vbo);
  glBufferData (GL_ARRAY_BUFFER, 20 * sizeof (float), points, GL_STATIC_DRAW);

  #ifndef EGL_RPI2
  if (shader_lvl >= 3) {
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float) ,(void *) ( 0 * sizeof(float) ) );
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) ( 3 * sizeof(float) ) );
  }
  #endif
  
  init();
  layers_init();
  glfwSetWindowRefreshCallback(win, render);
  glfwSetCursorPosCallback(win, cursor_pos_callback);
  glfwSetMouseButtonCallback(win, mouse_button_callback);
  glfwSetWindowSize( win, res[0], res[1] );
  glfwShowWindow( win );


  log_info("Starting Rendering\n");

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  struct timeval tv;

  gettimeofday(&tv, NULL);
  start_time = ((double) tv.tv_sec + ((double) tv.tv_usec / 1000000.0));

  while(!glfwWindowShouldClose(win)) {
    render(win);
    glfwPollEvents();
  }

  glfwTerminate();
  layers_destroy();
  return 0;             /* ANSI C requires main to return int. */
}
