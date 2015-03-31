// header missing
#include "config.h"
#ifdef MAC_OSX
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "shader.h"

extern int server_init(void);

extern float iGlobalTime;

extern float iResolution[2];

void
display(void)
{
  printf("iGlobalTime: %f\n", iGlobalTime);
  iGlobalTime += 0.01;
  iResolution[0] = 256;
  iResolution[1] = 256;

  glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //printf("applying shader layers\n");
  for(int i = 0; i < MAXSHADERLAYERS; i++)
  {
    applyShaderLayer(i);
  }

  removeDeadLayers();

  glutSwapBuffers();
  glutPostRedisplay();
}



void
init(void)
{
  server_init();
}

int
main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("red 3D lighted cube");
  glutDisplayFunc(display);
  glewInit();

  printf("GLEW version: s\n", glewGetString(GLEW_VERSION));
  printf("OpenGL version:  %s\n", glGetString(GL_VERSION));  
  init();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
