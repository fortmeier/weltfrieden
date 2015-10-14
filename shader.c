#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB

#define uarg(s, key, value) glUniform1f( glGetUniformLocation(s->progid, key), value )
#define uarg2fv(s, key, num, value) glUniform2fv( glGetUniformLocation(s->progid, key), num, value )


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#if defined(__linux)
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif

#include <GLFW/glfw3.h>


#include "shader.h"

GLuint vertex_shader;
GLuint vbo;
GLuint vao;

double now = 0;
float res[2];

int shader_lvl = 0;

shader* waiting = NULL;
shader* playing = NULL;

shader shaders[MAXSHADERLAYERS];

pthread_mutex_t layerlock;
pthread_mutex_t queuelock;

size_t read_file (FILE* file, char** content) {
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *content = malloc(fsize + 1);
  fread(*content, fsize, 1, file);

  (*content)[fsize] = 0;


  return fsize;
}

void dequeue(double now) {
  shader *p;
  pthread_mutex_lock(&queuelock);
  while ((p = queue_next(&waiting, now)) != NULL) {
#ifndef NDEBUG
    int s = queue_size(playing);
#endif

    p->prev = NULL;
    p->next = playing;
    if (playing != NULL) {
      playing->prev = p;
    }
    playing = p;
#ifndef NDEBUG
    assert(s == (queue_size(playing) - 1));
#endif
  }
  pthread_mutex_unlock(&queuelock);
}

void shader_from_cache(shader *s) {
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (shaders[i].state != UNUSED && strcmp(s->filename, shaders[i].filename) == 0)
      {
        s->progid = shaders[i].progid;
        s->shaderid = shaders[i].shaderid;
        break;
      }
  }
}

void shaders_init() {
  pthread_mutex_init(&layerlock, NULL);
  pthread_mutex_init(&queuelock, NULL);
}

void shaders_destroy() {
  pthread_mutex_destroy(&layerlock);
  pthread_mutex_destroy(&queuelock);
}

GLint shader_load( const char *filename, GLenum type ) {
  int infolength;
  char infolog[2048];
  char* content;

  GLint shader = glCreateShader( type);

  FILE* file = fopen(filename, "r");

  if( file == NULL ) {
    printf("file %s could not be opend\n", filename);
    exit(1);
  }

  long size = read_file( file, &content );

  fclose(file);

  const char *const *cContent = (const char * const*) (&content);

  glShaderSource( shader, 1, cContent, ((const int*)&size) );
  glCompileShader( shader );

  glGetShaderInfoLog( shader, 2048, &infolength, infolog );

  debug("shaderlog: %s\n", infolog);

  free (content);

  return shader;
}

// TODO: check for file's existance first before calling this
// avoid fatal exit during performance (most of the time)

void shader_init(shader* s) {
  if (s->progid == 0) {
    s->progid = glCreateProgram();

    char filename[256];

    sprintf(filename, "shaders/%s-%dxx.frag",
            s->filename,
            shader_lvl);

    s->shaderid = shader_load( filename, GL_FRAGMENT_SHADER );
    glAttachShader( s->progid, s->shaderid );

    sprintf(filename, "shaders/basic-%dxx.vert", shader_lvl);
    // TODO: cache this
    vertex_shader = shader_load(filename , GL_VERTEX_SHADER );
    glAttachShader( s->progid, vertex_shader );

    glLinkProgram( s->progid );

    int infolength;
    char infolog[2048];
    glGetProgramInfoLog( s->progid, 2048, &infolength, infolog );
  }
}

void shader_layer_destroy(shader* s) {
  s->state = UNUSED;
}


void map_play_args(shader* s) {
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

void shader_add(shader *s) {
  pthread_mutex_lock(&queuelock);
  queue_add(&waiting, s);
  pthread_mutex_unlock(&queuelock);
}

void shader_apply(shader *s) {
  if( s->state == UNINITIALIZED) {
    shader_init(s);
    debug("[shader:init] %s (%f vs %f)", s->filename, s->when, now);
    s->state = INITIALIZED;
  }
  if( s->state == INITIALIZED ) {
    if(s->when <= now) {
      glUseProgram( s->progid );

      uarg(s, "now", now);
      uarg(s, "elapsed", now - s->when);
      uarg2fv(s, "res", 1, res);

      map_play_args(s);


      GLint blendmode = GL_ONE_MINUS_SRC_ALPHA;
      switch(s->args.blendmode) {
      case NSA: blendmode = GL_ONE_MINUS_SRC_ALPHA; break;
      case NSC: blendmode = GL_ONE_MINUS_SRC_COLOR; break;
      case NDA: blendmode = GL_ONE_MINUS_DST_ALPHA; break;
      case NDC: blendmode = GL_ONE_MINUS_DST_COLOR; break;
      case SA: blendmode = GL_SRC_ALPHA; break;
      case SC: blendmode = GL_SRC_COLOR; break;
      case DA: blendmode = GL_DST_ALPHA; break;
      case DC: blendmode = GL_DST_COLOR; break;
      case SS: blendmode = GL_SRC_ALPHA_SATURATE; break;
      case CC: blendmode = GL_CONSTANT_COLOR; break;
      case CA: blendmode = GL_CONSTANT_ALPHA; break;
      }
      glBlendFunc(GL_SRC_ALPHA, blendmode);

      if (shader_lvl >= 3) {
        glBindVertexArray (vao);
      }
      else {
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
      }
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    }
  }
}


void shaders_apply() {
  dequeue(now);

  //  log_info("waiting queue size: %d @ %f\n", queue_size(waiting), now);
  shader *p = playing;

  while (p != NULL) {
    shader_apply(p);

    p = p->next;
  }
}


static void shader_reset(shader* s) {
  memset(s, 0, sizeof(shader));

  s->state = UNINITIALIZED;
  s->filename = NULL;
  s->filecontent = NULL;
  s->progid = 0;
  s->shaderid = 0;
  s->next = NULL;
  s->prev = NULL;
}

shader *shader_new()
{
  shader *result = NULL;
  pthread_mutex_lock(&layerlock);
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (shaders[i].state == UNUSED) {
      result = &shaders[i];
      shader_reset(&shaders[i]);
      break;
    }
  }
  pthread_mutex_unlock(&layerlock);
  return(result);
}

void
shaders_cleanup()
{
  shader* p = playing;

  while (p != NULL) {
    if ((p->when + p->duration) < now) {
      debug("removing: %s\n, played at: %f", p->filename, p->when);
      queue_remove(&playing, p);
    }
    p = p->next;
  }
  // log_info("playing queue size: %d\n", queue_size(playing));
}
