// header missing
#include "config.h"
#ifdef MAC_OSX
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif

extern int server_init(void);

float cube_angle = 60;

void
display(void)
{
//  printf("display!\n");

  float r,g,b;
  r= cube_angle;
  b = cube_angle;
  g = cube_angle;
  glClearColor(r,g,b,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1,0,0,1);
  glBegin(GL_QUADS);
  glVertex3f(0.2,0.2,0);
  glVertex3f(0.8,0.2,0);
  glVertex3f(0.8,0.8,0);
  glVertex3f(0.2,0.8,0);
  glEnd();

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
  init();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
