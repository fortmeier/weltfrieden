// header missing
#include "config.h"
/* #ifdef MAC_OSX */
/* //#include <GL/glew.h> */
/* #include <OpenGL/gl3.h> */
/* #include <GL/freeglut.h> */
/* #else */
/* #include <GL/glew.h> */
/* #include <GL/glut.h> */
/* #include <GL/gl.h> */
/* #endif */


#include <stdio.h>
#include <stdlib.h>

#if defined(__linux) || defined(_WIN32)
#  include <GLXW/glxw.h>
#endif

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <assert.h>

#include "shader.h"

GLuint vShader;
float iGlobalTime = 0;
float iResolution[2];


int activeShaders = 0;
shader shaderLayerArray[MAXSHADERLAYERS];

/* void checkGlError( const char* comment ) */
/* { */
/*   // printf("checked %s\n", comment); */
/*   GLenum error = glGetError(); */
/*   if( error != GL_NO_ERROR) */
/*   { */
/*     printf("gl error(%s): %s\n", comment, gluErrorString(error) ); */
/*     exit(-1); */
/*   } */
/* }  */



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
  //checkGlError( "glCreateShader" );

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
  //checkGlError( "shader source" );

  glCompileShader( shader );
  //checkGlError( "compile shader" );
  long infolength;
  char* infolog[2048];
  glGetShaderInfoLog( shader, 2048, &infolength, infolog );
  printf("shaderlog: %s\n", infolog);
  return shader;
}

void
initShaderLayer(shader* s)
{
  s->progId = glCreateProgram();

  char filename[256];

  sprintf(filename, "shaders/%s", s->filename);

  s->shaderId = loadShader( filename, GL_FRAGMENT_SHADER );
  glAttachShader( s->progId, s->shaderId );
  //checkGlError("attachShader");

  vShader = loadShader( "shaders/basic.vert", GL_VERTEX_SHADER );
  glAttachShader( s->progId, vShader );
  //checkGlError("attachVShader");

  glLinkProgram( s->progId );
  //  checkGlError("link program");

  long infolength;
  char* infolog[2048];
  glGetProgramInfoLog( s->progId, 2048, &infolength, infolog );
  printf("log: %s\n", infolog);

  glUseProgram( s->progId );
  //  checkGlError( "use program" );

  glGetProgramInfoLog( s->progId, 2048, &infolength, infolog );
  printf("log: %s\n", infolog);
  //glUseProgram( 0 );
  //  checkGlError("useprog0");
}

void
deinitShaderLayer(shader* s)
{
  printf("remove shader\n");
  s->state = UNUSED;
}

void
addShaderLayer(shader s)
{
  assert("too many layers" && activeShaders < MAXSHADERLAYERS-1);
  printf("adding shader layer %i ending at %f\n", activeShaders, iGlobalTime);
  shaderLayerArray[activeShaders] = s;
  activeShaders++;
}

void
useShaderLayer(shader *s)
{
  if( s->state == UNINITIALIZED)
  {
    printf("initializing shader layer\n");
    initShaderLayer(s);
    s->state = INITIALIZED;
  }
  if( s->state == INITIALIZED )
  {
    //    printf(" applying shader .. \n" );
    glUseProgram( s->progId );

    GLint timelocation = glGetUniformLocation( s->progId, "iGlobalTime");
    glUniform1f(timelocation, iGlobalTime);
    //    checkGlError("uniform time");

    GLint resolocation = glGetUniformLocation( s->progId, "iResolution");
    glUniform2fv(resolocation, 1,  iResolution);
    //    checkGlError("uniform resolution");

    GLint gainlocation = glGetUniformLocation( s->progId, "gain");
    glUniform1f(gainlocation, s->gain);
    ///    checkGlError("uniform resolution");
    /*
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(0,0,0);
    glTexCoord2f(1,0); glVertex3f(1.0,0,0);
    glTexCoord2f(1,1); glVertex3f(1.0,1,0);
    glTexCoord2f(0,1); glVertex3f(0,1,0);
    glEnd();
*/
  }
}

void
applyShaderLayer(unsigned int i)
{
  assert("layer out of bounds" && i >= 0 && i < MAXSHADERLAYERS);
  useShaderLayer( &shaderLayerArray[i] );
}

void
removeDeadLayers()
{
  // remove dead layers
  for(int i = 0; i < MAXSHADERLAYERS; i++)
  {
    if( shaderLayerArray[i].state == INITIALIZED && shaderLayerArray[i].end < iGlobalTime)
    {
      deinitShaderLayer( &shaderLayerArray[i] );
      for( int j = i; j < MAXSHADERLAYERS-1; j++ )
      {
        shaderLayerArray[j] = shaderLayerArray[j+1];
      }
      shaderLayerArray[MAXSHADERLAYERS-1].state = UNUSED;
      activeShaders--;
    }

  }
}
