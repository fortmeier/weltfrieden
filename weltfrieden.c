
// header missing
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <getopt.h>

#include "dbg.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#if defined(__linux)
#include <GL/glut.h>
#endif

#include <assert.h>

#include "shader.h"

extern int server_init(void);

extern float iGlobalTime;

extern float iResolution[2];

double epochOffset = 0;

extern GLuint vao;

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
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
  glfwWindowHint(GLFW_FLOATING, GL_TRUE);
  GLFWwindow* win = NULL;
  int w = 256;
  int h = 256;


  win = glfwCreateWindow(w, h, "GLFW", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(win);

  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString (GL_VERSION); // version as a string
  log_info("Renderer: %s\n", renderer);
  log_info("OpenGL version supported %s\n", version);

  glfwSwapInterval(1);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLuint vbo = 0;
  glGenBuffers (1, &vbo);
  glBindBuffer (GL_ARRAY_BUFFER, vbo);
  glBufferData (GL_ARRAY_BUFFER, 12 * sizeof (float), points, GL_STATIC_DRAW);

  glGenVertexArrays (1, &vao);
  glBindVertexArray (vao);
  glEnableVertexAttribArray (0);
  glBindBuffer (GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


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
    //  glUseProgram(0);
  }

  glfwTerminate();
  uninitShaders();
  return 0;             /* ANSI C requires main to return int. */
}
