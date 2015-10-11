// header missing
#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#if defined(__linux)
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif

#define GLFW_INCLUDE_GLEXT
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <assert.h>

#include "dbg.h"
#include "shader.h"

GLuint vShader;
float iGlobalTime = 0;
float iResolution[2];
int shading_lang_lvl = 0;
GLuint vbo;
GLuint vao;

shader* waiting = NULL;
shader* playing = NULL;


int activeShaders = 0;
shader shaders[MAXSHADERLAYERS];

pthread_mutex_t shaders_lock;

pthread_mutex_t queue_waiting_lock;

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


int queue_size(shader *queue) {
  int result = 0;
  while (queue != NULL) {
    result++;
    queue = queue->next;
    if (result > 4096) {
      printf("whoops, big queue\n");
      break;
    }
  }
  return(result);
}

void queue_add(shader **queue, shader *new) {
  //  printf("queuing %s @ %f\n", new->filename, new->when);
  int added = 0;
  if (*queue == NULL) {
    *queue = new;
    added++;
  }
  else {
    shader *tmp = *queue;
    assert(tmp->prev == NULL);

    int i =0;
    while (1) {
      if (tmp->when > new->when) {
        // insert in front of later event
        new->next = tmp;
        new->prev = tmp->prev;
        if (new->prev != NULL) {
          new->prev->next = new;
        }
        else {
          *queue = new;
        }
        tmp->prev = new;

        added++;
        break;
      }

      if (tmp->next == NULL) {
        // add to end of queue
        tmp->next = new;
        new->prev = tmp;
        added++;
        break;
      }
      ++i;
      tmp = tmp->next;
    }
  }

  assert(added == 1);
}


void queue_remove(shader **queue, shader *old) {
  if (old->prev == NULL) {
    *queue = old->next;
    if (*queue  != NULL) {
      (*queue)->prev = NULL;
    }
  }
  else {
    old->prev->next = old->next;

    if (old->next) {
      old->next->prev = old->prev;
    }
  }
  old->state = UNUSED;
}

shader *queue_next(shader **queue, float now) {
  shader *result = NULL;
  //  printf("%f vs %f\n", *queue == NULL ? 0 : (*queue)->when, now);
  if (*queue != NULL && (*queue)->when <= now) {
    result = *queue;
    *queue = (*queue)->next;
    if ((*queue) != NULL) {
      (*queue)->prev = NULL;
    }
  }

  return(result);
}

void dequeue(float now) {
  shader *p;
  pthread_mutex_lock(&queue_waiting_lock);
  while ((p = queue_next(&waiting, now)) != NULL) {
/* #ifdef DEBUG */
/*     int s = queue_size(playing); */
/* #endif */

//    cut(p);
    p->prev = NULL;
    p->next = playing;
    if (playing != NULL) {
      playing->prev = p;
    }
    playing = p;
/* #ifdef DEBUG */
/*     assert(s == (queue_size(playing) - 1)); */
/* #endif */

    //printf("done.\n");
  }
  pthread_mutex_unlock(&queue_waiting_lock);
}

void loadFromCache(shader *s) {
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (shaders[i].state != UNUSED && strcmp(s->filename, shaders[i].filename) == 0) {
      s->progId = shaders[i].progId;
      s->shaderId = shaders[i].shaderId;
      break;
    }
  }
}

void initShaders()
{
  pthread_mutex_init(&shaders_lock, NULL);
  pthread_mutex_init(&queue_waiting_lock, NULL);
}

void uninitShaders()
{
  pthread_mutex_destroy(&shaders_lock);
  pthread_mutex_destroy(&queue_waiting_lock);
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

  fclose(file);

  const char *const *cContent = (const char * const*) (&content);
  glShaderSource( shader, 1, cContent, ((const int*)&size) );

  glCompileShader( shader );

  int infolength;
  char infolog[2048];
  glGetShaderInfoLog( shader, 2048, &infolength, infolog );
  debug("shaderlog: %s\n", infolog);
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

    sprintf(filename, "shaders/%s-%dxx.frag",
            s->filename,
            shading_lang_lvl);

    s->shaderId = loadShader( filename, GL_FRAGMENT_SHADER );
    glAttachShader( s->progId, s->shaderId );

    sprintf(filename, "shaders/basic-%dxx.vert", shading_lang_lvl);
    vShader = loadShader(filename , GL_VERTEX_SHADER );
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
addShaderLayer(shader *s)
{
  //activeShaders = activeShaders % MAXSHADERLAYERS;

  //  loadFromCache(&s);


  //deinitShaderLayer(&shaders[activeShaders]);
  // shaders[MAXSHADERLAYERS-1].state = UNUSED;

  //  shaders[activeShaders] = s;
  pthread_mutex_lock(&queue_waiting_lock);
  queue_add(&waiting, s);
  pthread_mutex_unlock(&queue_waiting_lock);
  //activeShaders++;
}

void
useShaderLayer(shader *s)
{
  if( s->state == UNINITIALIZED)
  {
    initShaderLayer(s);
    debug("[shader:init] %s (%f)", s->filename, s->args.gain);
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

    if (shading_lang_lvl >= 3) {
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


void applyShaderLayers() {
  dequeue(iGlobalTime);

  //  log_info("waiting queue size: %d\n", queue_size(waiting));
  shader *p = playing;

  while (p != NULL) {
    /* if (p->when > iGlobalTime) { */
    /*   p = p->next; */
    /*   continue; */
    /* } */

    useShaderLayer(p);

    p = p->next;
  }
}

void
applyShaderLayer(unsigned int i)
{
  assert("layer out of bounds" && i >= 0 && i < MAXSHADERLAYERS);
  useShaderLayer( &shaders[i] );
}

static void reset_shader(shader* s) {
  memset(s, 0, sizeof(shader));

  s->state = UNINITIALIZED;
  s->filename = NULL;
  s->filecontent = NULL;
  s->progId = 0;
  s->shaderId = 0;
  s->next = NULL;
  s->prev = NULL;

}

shader *new_shader()
{
  shader *result = NULL;
  pthread_mutex_lock(&shaders_lock);
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (shaders[i].state == UNUSED) {
      result = &shaders[i];
      reset_shader(&shaders[i]);
      break;
    }
  }
  pthread_mutex_unlock(&shaders_lock);
  return(result);
}

void
removeDeadLayers()
{
  shader* p = playing;

  while (p != NULL) {
    if ((p->when + p->duration) < iGlobalTime) {
      //      debug("removing: %s\n, played at: %f", p->filename, p->when);
      queue_remove(&playing, p);
    }
    p = p->next;
  }

  //  log_info("playing queue size: %d\n", queue_size(playing));

  /* // remove dead layers */
  /* for(int i = 0; i < MAXSHADERLAYERS; i++) */
  /* { */
  /*   if( shaders[i].state == INITIALIZED && (shaders[i].when + shaders[i].duration) < iGlobalTime) */
  /*   { */
  /*     deinitShaderLayer( &shaders[i] ); */
  /*     if (activeShaders > 0) { */
  /*       activeShaders--; */
  /*     } */
  /*   } */
  /* } */
}
