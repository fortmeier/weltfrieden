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
extern int server_init(void);

float cube_angle = 60;

GLuint fbo = 0;
GLuint texId = 0;
GLuint dbo = 0;

GLuint prog = 0;

GLuint diffShader = 0;
GLuint vShader = 0;

GLint iGlobalTime = 0.5;
GLint iResolution = 0;

GLfloat time = 0.0;
float resu[2] = {256.0, 256.0};

void checkGlError( const char* comment )
{
  // printf("checked %s\n", comment);
  GLenum error = glGetError();
  if( error != GL_NO_ERROR)
  {
    printf("gl error(%s): %s\n", comment, gluErrorString(error) );
    exit(-1);
  }
} 

void
display(void)
{
//  printf("display!\n");

  glUniform2f(iResolution,resu[0],resu[1]);
	  checkGlError("uniform ires");
  float r,g,b;
  r = cube_angle;
  b = cube_angle;
  g = cube_angle;
  
#if 0
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

  glClearColor(r,g,b,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //glLinkProgram( prog );
  //checkGlError( "linkprog1" );
  //glUseProgram( prog );
  //checkGlError("useprog1");
 
  glColor4f(1,0,0,1);
  glBegin(GL_QUADS);
  glVertex3f(0.2,0.2,0);
  glVertex3f(0.8,0.2,0);
  glVertex3f(0.8,0.8,0);
  glVertex3f(0.2,0.8,0);
  glEnd();

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
#endif
 glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_TEXTURE_2D);
  //glBindTexture(GL_TEXTURE_2D, texId);
  checkGlError("bindtex");

  //glUseProgram( prog );
  checkGlError("useprog2");
  glUniform1f(iGlobalTime,time);
  checkGlError("univ");
  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, 1, 0, 1);  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f(1,1,1,1);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(0,0,0);
  glTexCoord2f(1,0); glVertex3f(1.0,0,0);
  glTexCoord2f(1,1); glVertex3f(1.0,1,0);
  glTexCoord2f(0,1); glVertex3f(0,1,0);
  glEnd();

  time += 0.01; 

  glutSwapBuffers();
  glutPostRedisplay();
}


size_t readFile(FILE* file, char** content)
{
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *content = malloc(fsize + 1);
  fread(*content, fsize, 1, file);

  (*content)[fsize] = 0;

  
  return fsize;
}

GLint loadShader( const char *filename, GLenum type )
{
  printf("opening shader file %s\n",filename);
  GLint shader = glCreateShader( type);
  checkGlError( "glCreateShader" );

  FILE* file = fopen(filename, "r");

  if( file == NULL )
  {
    printf("file %s could not be opend\n", filename);
    exit(1);
  }
 

  char* content;
  
  long size = readFile( file, &content ); 
  printf("filesize: %i", size);
  printf(" .. success\n");

  //printf("%s", content);
  
  fclose(file);
  glShaderSource( shader, 1, &content, &size );
  checkGlError( "shader source" );

  glCompileShader( shader );
  checkGlError( "compile shader" );
  long infolength;
  char* infolog[2048];
  glGetShaderInfoLog( shader, 2048, &infolength, infolog );
  printf("shaderlog: %s\n", infolog);
  return shader;
}

void
initShader(GLuint *shader, const char* filename)
{
  prog = glCreateProgram();
  
  *shader = loadShader( filename, GL_FRAGMENT_SHADER );
  glAttachShader( prog, *shader );
  checkGlError("attachShader");

  //vShader = loadShader( "shaders/basic.vert", GL_VERTEX_SHADER );
  //glAttachShader( prog, vShader );
  //checkGlError("attachVShader");

  glLinkProgram( prog );
  checkGlError("link program");

  long infolength;
  char* infolog[2048];
  glGetProgramInfoLog( prog, 2048, &infolength, infolog );
  printf("log: %s\n", infolog);
 
  glUseProgram( prog );
  checkGlError( "use program" );

  iGlobalTime = glGetUniformLocation(prog,"iGlobalTime");
  checkGlError("add uniform");
  glUniform1f(iGlobalTime,time);
  
  iResolution = glGetUniformLocation(prog,"iResolution");
  checkGlError("add uniform");
  glUniform2f(iResolution,resu[0],resu[1]);
  printf("%f check\n", resu[0]);
	  checkGlError("set resolution");

  printf("uniform %i\n", iGlobalTime); 
  printf("uniform %i\n", iResolution); 

  //float x[1];
  //glGetUniformfv(prog, iGlobalTime, 1, &x);
  //printf("x = %f\n", x );

  glGetProgramInfoLog( prog, 2048, &infolength, infolog );
  printf("log: %s\n", infolog);
  //glUseProgram( 0 );
  checkGlError("useprog0");
}

void
init(void)
{
  // init fbo
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

  glGenFramebuffersEXT(1, &fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
   //Attach 2D texture to this FBO
   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texId, 0);

  glGenRenderbuffersEXT(1, &dbo );
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dbo );
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, 256, 256);

  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, dbo);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  // init the diffusive shader

  initShader( &diffShader, "shaders/basic.frag");
  //initShader( &diffShader, "shaders/diffusive.glsl");
  
  // init the 
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
