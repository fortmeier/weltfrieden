
// header missing
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(__linux) || defined(_WIN32)
#  include <GLXW/glxw.h>
#endif

//#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

/* #define ROXLU_USE_MATH */
/* #define ROXLU_USE_PNG */
/* #define ROXLU_USE_OPENGL */
/* #define ROXLU_IMPLEMENTATION */
//#include <tinylib.h>

#include <assert.h>

#include "shader.h"

extern int server_init(void);

extern float iGlobalTime;

extern float iResolution[2];

float points[] = {
  1.0, 1.0, 0.0f,
 -1.0, 1.0, 0.0f,
  1.0, -1.0, 0.0f,
 -1.0, -1.0, 0.0f,
};



/* void */
/* display(void) */
/* { */
/*   printf("iGlobalTime: %f\n", iGlobalTime); */
/*   iGlobalTime += 0.01; */
/*   iResolution[0] = 256; */
/*   iResolution[1] = 256; */

/*   glClearColor(0.0,0.0,0.0,1.0); */
/*   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
/*   glDisable(GL_DEPTH_TEST); */
/*   glEnable(GL_BLEND); */
/*   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */
/*   glMatrixMode(GL_PROJECTION); */
/*   glLoadIdentity(); */
/*   gluOrtho2D(0, 1, 0, 1);   */
/*   glMatrixMode(GL_MODELVIEW); */
/*   glLoadIdentity(); */

/*   //printf("applying shader layers\n"); */
/*   for(int i = 0; i < MAXSHADERLAYERS; i++) */
/*   { */
/*     applyShaderLayer(i); */
/*   } */

/*   removeDeadLayers(); */

/*   glutSwapBuffers(); */
/*   glutPostRedisplay(); */
/* } */



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

  GLFWwindow* win = NULL;
  int w = 640;
  int h = 480;


  win = glfwCreateWindow(w, h, "GLFW", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  /* glfwSetFramebufferSizeCallback(win, resize_callback); */
  /* glfwSetKeyCallback(win, key_callback); */
  /* glfwSetCharCallback(win, char_callback); */
  /* glfwSetCursorPosCallback(win, cursor_callback); */
  /* glfwSetMouseButtonCallback(win, button_callback); */
  glfwMakeContextCurrent(win);


  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString (GL_VERSION); // version as a string
  printf ("Renderer: %s\n", renderer);
  printf ("OpenGL version supported %s\n", version);


  glfwSwapInterval(1);

  //  glViewport(0, 0, (GLsizei)w, (GLsizei)h);

  //glClearColor(0.0,0.0,0.0,1.0);
  //    glEnable(GL_TEXTURE_2D);
  /*    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();*/
    /* gluOrtho2D(0, 1, 0, 1);   */
  /*    glOrtho(0, w, h, 0, 0, 1);*/


GLuint vbo = 0;
glGenBuffers (1, &vbo);
glBindBuffer (GL_ARRAY_BUFFER, vbo);
glBufferData (GL_ARRAY_BUFFER, 12 * sizeof (float), points, GL_STATIC_DRAW);

 GLuint vao = 0;
glGenVertexArrays (1, &vao);
glBindVertexArray (vao);
glEnableVertexAttribArray (0);
glBindBuffer (GL_ARRAY_BUFFER, vbo);
glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


#if defined(__linux) || defined(_WIN32)
  if(glxwInit() != 0) {
    printf("Error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }
#endif
  init();
  printf("Starting Rendering\n");
  // ----------------------------------------------------------------
  // THIS IS WHERE YOU START CALLING OPENGL FUNCTIONS, NOT EARLIER!!
  // ----------------------------------------------------------------

  while(!glfwWindowShouldClose(win)) {


    // printf("iGlobalTime: %f\n", iGlobalTime);
    iGlobalTime += 0.01;
    iResolution[0] = 256;
    iResolution[1] = 256;



    /*    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); */
  glBindVertexArray (vao);
  // draw points 0-3 from the currently bound VAO with current in-use shader
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  // update other events like input handling
  glfwPollEvents ();
    //printf("applying shader layers\n");
    for(int i = 0; i < MAXSHADERLAYERS; i++)
      {
	applyShaderLayer(i);
      }

    removeDeadLayers();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glfwTerminate();



  /* glutInit(&argc, argv); */
  /* #ifdef MAC_OSX */
  /* glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); */
  /* #else */
  /* glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); */
  /* #endif */
  /* //glutInitContextVersion(3, 2); */
  /* glutInitContextFlags(GLUT_CORE_PROFILE); */
  /* glutCreateWindow("red 3D lighted cube"); */
  /* glutDisplayFunc(display); */
  /* //  glewInit(); */

  /* //  printf("GLEW version: %s\n", glewGetString(GLEW_VERSION)); */
  /* printf("OpenGL version:  %s\n", glGetString(GL_VERSION));   */

  /* glutMainLoop(); */
  return 0;             /* ANSI C requires main to return int. */
}
