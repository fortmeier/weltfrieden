
// header missing
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <getopt.h>

#include "dbg.h"

#if defined(__linux)
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif

#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <assert.h>

#include "shader.h"

extern int server_init(void);

extern float iGlobalTime;

extern float iResolution[2];

double epochOffset = 0;

extern GLuint vbo;
extern GLuint vao;

extern int shading_lang_lvl;

float points[] = {
  1.0, 1.0, 0.0f,
 -1.0, 1.0, 0.0f,
  1.0, -1.0, 0.0f,
 -1.0, -1.0, 0.0f,
};

void
init(void)
{
  server_init();

}

void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}


int
main(int argc, char **argv)
{
  /* int c; */
  /* static int cache_flag; */

  /* while (1) { */
  /*   static struct option long_options[] = */
  /*     { */
  /*       {"cache", no_argument, &cache_flag, 1}, */

  /*       {"version", no_argument, 0, 'v'}, */
  /*       {"help",    no_argument, 0, 'h'}, */
  /*       {0,0,0,0} */
  /*     }; */

  /*   int option_index = 0; */

  /*   c = getopt_long(argc, argv, "n:", long_options, &option_index); */

  /*   if (c == -1) { */
  /*     break; */
  /*   } */

  /*   switch (c) { */
  /*   case 0: */
  /*     if (long_options[option_index].flag != 0) break; */
  /*   case 'v': */
  /*     log_info("Version: 0.0.0, not yet implemented"); */
  /*     break; */
  /*   case 'h': */
  /*     log_info("Usage: ./weltfrieden [--cache]"); */
  /*     break; */
  /*   default: */
  /*     return 1; */
  /*   } */
  /* } */



  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    return 1;
  }

  //glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_FLOATING, GL_TRUE);
  GLFWwindow* win = NULL;
  int w = 256;
  int h = 256;

  shading_lang_lvl = 0;

  win = glfwCreateWindow(w, h, "GLFW", NULL, NULL);
  if(!win) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    log_info("Falling back to OpenGL 2.1\n");
    shading_lang_lvl = 1;
    win = glfwCreateWindow(w, h, "GLFW", NULL, NULL);
    if (!win) {
      glfwTerminate();
      exit(EXIT_FAILURE);
    }
  }
  else {
    shading_lang_lvl = 3;
  }

  glfwMakeContextCurrent(win);

  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString (GL_VERSION); // version as a string
  log_info("Renderer: %s\n", renderer);
  log_info("OpenGL version supported %s\n", version);

  log_info("Shading Language Level: %dxx", shading_lang_lvl);

  glfwSwapInterval(1);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glGenBuffers (1, &vbo);
  glBindBuffer (GL_ARRAY_BUFFER, vbo);
  glBufferData (GL_ARRAY_BUFFER, 12 * sizeof (float), points, GL_STATIC_DRAW);

  if (shading_lang_lvl >= 3) {
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  }

  init();

  struct timeval tv;
  gettimeofday(&tv, NULL);
  epochOffset = ((double) tv.tv_sec + ((double) tv.tv_usec / 1000000.0));

  log_info("Starting Rendering\n");

  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  initShaders();

  while(!glfwWindowShouldClose(win)) {
    iGlobalTime = glfwGetTime();

    iResolution[0] = w;
    iResolution[1] = h;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i = 0; i < MAXSHADERLAYERS; i++)
      {
	applyShaderLayer(i);
      }

    removeDeadLayers();
    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glfwTerminate();
  uninitShaders();
  return 0;             /* ANSI C requires main to return int. */
}
