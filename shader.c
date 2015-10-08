// header missing
#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
GLuint vao;

int activeShaders = 0;
shader shaderLayerArray[MAXSHADERLAYERS];

pthread_mutex_t shaders_lock;


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

void loadFromCache(shader *s) {
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (shaderLayerArray[i].state != UNUSED && strcmp(s->filename, shaderLayerArray[i].filename) == 0) {
      s->progId = shaderLayerArray[i].progId;
      s->shaderId = shaderLayerArray[i].shaderId;
      //      printf("loaded %s from cache id: %d\n", s->filename, s->progId);
      break;
    }
  }
}

void initShaders()
{
  pthread_mutex_init(&shaders_lock, NULL);
}

void uninitShaders()
{
  pthread_mutex_destroy(&shaders_lock);
}

GLint loadShader( const char *filename, GLenum type )
{
  GLint shader = glCreateShader( type);

  FILE* file = fopen(filename, "r");

  if( file == NULL )
    {
      printf("file %s could not be opend\n", filename);
      exit(1);
    }

  char* content;

  long size = readFile( file, &content );
  //  printf("filesize: %i", size);
  //  printf(" .. success\n");

  fclose(file);

  const char *const *cContent = (const char * const*) (&content);
  glShaderSource( shader, 1, cContent, ((const int*)&size) );

  glCompileShader( shader );

  int infolength;
  char infolog[2048];
  glGetShaderInfoLog( shader, 2048, &infolength, infolog );
  //  printf("shaderlog: %s\n", infolog);
  free (content);
  return shader;
}

// TODO: check for file's existance first before calling this
// avoid fatal exit during performance (most of the time)

void
initShaderLayer(shader* s)
{
  if (s->progId == 0) {
    s->progId = glCreateProgram();

    char filename[256];

    sprintf(filename, "shaders/%s.frag", s->filename);

    s->shaderId = loadShader( filename, GL_FRAGMENT_SHADER );
    glAttachShader( s->progId, s->shaderId );

    vShader = loadShader( "shaders/basic.vert", GL_VERTEX_SHADER );
    glAttachShader( s->progId, vShader );

    glLinkProgram( s->progId );

    int infolength;
    char infolog[2048];
    glGetProgramInfoLog( s->progId, 2048, &infolength, infolog );
  }
}

void
deinitShaderLayer(shader* s)
{
  s->state = UNUSED;
}

void
addShaderLayer(shader s)
{
  activeShaders = activeShaders % MAXSHADERLAYERS;

  loadFromCache(&s);


  deinitShaderLayer(&shaderLayerArray[activeShaders]);
  shaderLayerArray[MAXSHADERLAYERS-1].state = UNUSED;

  shaderLayerArray[activeShaders] = s;
  activeShaders++;
}

void
useShaderLayer(shader *s)
{
  if( s->state == UNINITIALIZED)
  {
    initShaderLayer(s);
    s->state = INITIALIZED;
  }
  if( s->state == INITIALIZED )
  {
    glUseProgram( s->progId );

    GLint globaltimelocation = glGetUniformLocation( s->progId, "iGlobalTime");
    glUniform1f(globaltimelocation, iGlobalTime);

    GLint timelocation = glGetUniformLocation( s->progId, "iTime");
    glUniform1f(timelocation, iGlobalTime - s->when);


    GLint resolocation = glGetUniformLocation( s->progId, "iResolution");
    glUniform2fv(resolocation, 1,  iResolution);

    GLint gainlocation = glGetUniformLocation( s->progId, "gain");
    glUniform1f(gainlocation, s->args.gain);

    GLint shapelocation = glGetUniformLocation( s->progId, "shape");
    glUniform1f(shapelocation, s->args.shape);

    GLint speedlocation = glGetUniformLocation( s->progId, "speed");
    glUniform1f(speedlocation, s->args.speed);

    GLint beginlocation = glGetUniformLocation( s->progId, "begin");
    glUniform1f(beginlocation, s->args.start);

    GLint endlocation = glGetUniformLocation( s->progId, "end");
    glUniform1f(endlocation, s->args.end);

    GLint offsetlocation = glGetUniformLocation( s->progId, "offset");
    glUniform1f(offsetlocation, s->args.offset);

    glBindVertexArray (vao);

    GLint blend_mode = GL_ONE_MINUS_SRC_ALPHA;
    switch(s->args.blend_mode) {
    case NSA: blend_mode = GL_ONE_MINUS_SRC_ALPHA; break;
    case NSC: blend_mode = GL_ONE_MINUS_SRC_COLOR; break;
    case NDA: blend_mode = GL_ONE_MINUS_DST_ALPHA; break;
    case NDC: blend_mode = GL_ONE_MINUS_DST_COLOR; break;
    case SA: blend_mode = GL_SRC_ALPHA; break;
    case SC: blend_mode = GL_SRC_COLOR; break;
    case DA: blend_mode = GL_DST_ALPHA; break;
    case DC: blend_mode = GL_DST_COLOR; break;
    case SS: blend_mode = GL_SRC_ALPHA_SATURATE; break;
    case CC: blend_mode = GL_CONSTANT_COLOR; break;
    case CA: blend_mode = GL_CONSTANT_ALPHA; break;
    }
    glBlendFunc(GL_SRC_ALPHA, blend_mode);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
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
    if( shaderLayerArray[i].state == INITIALIZED && (shaderLayerArray[i].when + shaderLayerArray[i].duration) < iGlobalTime)
    {
      deinitShaderLayer( &shaderLayerArray[i] );
      if (activeShaders > 0) {
        activeShaders--;
      }
    }

  }
}
