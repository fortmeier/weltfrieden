// header missing
#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

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
  printf("shaderlog: %s\n", infolog);
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

#define uarg(s, key, value) glUniform1f( glGetUniformLocation(s->progId, key), value )
#define uarg2fv(s, key, num, value) glUniform2fv( glGetUniformLocation(s->progId, key), num, value )

void mapPlayArgs(shader* s) {
  uarg(s, "cps", s->args.cps);
  uarg(s, "offset", s->args.offset);
  uarg(s, "start", s->args.start);
  uarg(s, "end", s->args.end);
  uarg(s, "speed", s->args.speed);
  uarg(s, "pan", s->args.pan);
  uarg(s, "velocity", s->args.velocity);
  uarg(s, "cutoff", s->args.cutoff);
  uarg(s, "resonance", s->args.resonance);
  uarg(s, "accelerate", s->args.accelerate);
  uarg(s, "shape", s->args.shape);
  uarg(s, "gain", s->args.gain);
  uarg(s, "delay", s->args.delay);
  uarg(s, "delaytime", s->args.delaytime);
  uarg(s, "delayfeedback", s->args.delayfeedback);
  uarg(s, "crush", s->args.crush);
  uarg(s, "coarse", s->args.coarse);
  uarg(s, "hcutoff", s->args.hcutoff);
  uarg(s, "hresonance", s->args.hresonance);
  uarg(s, "bandf", s->args.bandf);
  uarg(s, "bandq", s->args.bandq);
}

void
addShaderLayer(shader s)
{
  activeShaders = activeShaders % MAXSHADERLAYERS;

  //  loadFromCache(&s);


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
    if(s->when <= iGlobalTime) {
    glUseProgram( s->progId );

    uarg(s, "iGlobalTime", iGlobalTime);
    uarg(s, "iTime", iGlobalTime - s->when);
    uarg2fv(s, "iResolution", 1, iResolution);

    mapPlayArgs(s);

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
    //  debug("playing at %f\n", iGlobalTime);
    }
    /* else { */
    /*   debug("not yet playing: %f %f", iGlobalTime, s->when); */
    /* } */
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
