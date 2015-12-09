#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "layers.h"
#include "thpool.h"
#include "shader.h"

GLuint sampler = 0;
GLuint* fbo = 0;

layer* waiting = NULL;
layer* showing = NULL;

static int pass = 0;

layer layers[MAXSHADERLAYERS];

pthread_mutex_t layerlock;
pthread_mutex_t queuelock;

thpool_t* read_file_pool;

extern int shader_lvl;
extern double now;
extern float res[2];
extern GLuint *texfbo;
extern GLuint text_vshader;
extern GLuint text_shader;
extern GLenum draw_buffer;
extern int cache;

GLuint vbo;
GLuint vao;

void dequeue(double now) {
  layer *p;
  pthread_mutex_lock(&queuelock);
  if (showing == NULL) {
    p = queue_next(&waiting, now);

    if (p == NULL) {
      pthread_mutex_unlock(&queuelock);
      return;
    }
    else {
      p->next = NULL;
      p->prev = NULL;

      showing = p;
    }
  }
  while ((p = queue_next(&waiting, now)) != NULL) {
#ifndef NDEBUG
    int s = queue_size(showing);
#endif
    if (showing->level >= p->level) {
      p->prev = NULL;
      p->next = showing;
      if (showing != NULL) {
        showing->prev = p;
      }
      showing = p;
    }
    else {
      layer *tmp = showing;

      while (1) {
        if (tmp->level >= p->level) {
          // insert p in between prev/next
          p->prev = tmp->prev;
          p->next = tmp;

          if (p->prev != NULL) {
            p->prev->next = p;
          }
          else {
            showing = p;
          }

          tmp->prev = p;
          break;
        }

        if (tmp->next == NULL) {
          p->next = NULL;
          tmp->next = p;
          p->prev = tmp;
          break;
        }

        tmp = tmp->next;
      }
    }
#ifndef NDEBUG
    assert(s == (queue_size(showing) - 1));
#endif
  }
  pthread_mutex_unlock(&queuelock);
}


void layers_destroy() {
  pthread_mutex_destroy(&layerlock);
  pthread_mutex_destroy(&queuelock);

  if (read_file_pool) {
    thpool_destroy(read_file_pool);
  }
}

void layers_finish(int even) {
}


void layers_apply() {
  dequeue(now);

  layer *p = showing;

  int run = 0;
  while (p != NULL) {
    layer_apply(p, pass);
    pass = (pass == 0) ? 1 : 0;

    p = p->next;
    run++;
  }
  if (run > 0) {
    pass = (pass == 0) ? 1 : 0;

    layers_finish(pass);
    pass = (pass == 0) ? 1 : 0;

  }
}

void layers_cleanup() {
  layer* p = showing;

  while (p != NULL) {
    if ((p->when + (p->duration/p->cps)) < now) {
      queue_remove(&showing, p);
      p->state = SHOWN;
    }
    p = p->next;
  }
}

void layers_clear_cache() {
  pthread_mutex_lock(&layerlock);
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    int state = layers[i].state;

    switch (state) {
    case INITIALIZED:
      layers[i].state = UNINITIALIZED;
      break;
    case SHOWN:
      layers[i].state = UNUSED;
      break;
    }
  }
  pthread_mutex_unlock(&layerlock);
}

void layers_init(int num_workers) {
  read_file_pool = thpool_init(num_workers);
  log_info("[shaders] init (cache: %d)\n", cache);
  pthread_mutex_init(&layerlock, NULL);
  pthread_mutex_init(&queuelock, NULL);

  shaderlayer_init_noise();
}
