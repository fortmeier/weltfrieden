#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "layers.h"
#include "text.h"
#include "shader.h"

GLuint sampler = 0;
GLuint* fbo = 0;

layer* waiting = NULL;
layer* showing = NULL;

static int pass = 0;

layer layers[MAXSHADERLAYERS];

pthread_mutex_t layerlock;
pthread_mutex_t queuelock;

extern int shader_lvl;
extern double now;
extern float res[2];
extern GLuint *texfbo;
extern GLuint text_vshader;
extern GLuint text_shader;
extern GLenum draw_buffer;
extern int cache;

static GLuint fbo_shader = 0;
static GLuint fbo_progid = 0;

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
#ifndef NODEBUG
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
#ifndef NODEBUG
    assert(s == (queue_size(showing) - 1));
#endif
  }
  pthread_mutex_unlock(&queuelock);
}


void layers_destroy() {
  pthread_mutex_destroy(&layerlock);
  pthread_mutex_destroy(&queuelock);
}

void layers_finish(int even) {
  /*  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  if (cache == 0 || fbo_shader == 0) {
    char filename[256];
    sprintf(filename, "shaders/passthru-%dxx.frag",
            shader_lvl);

    fbo_shader = _shader_load(filename, GL_FRAGMENT_SHADER);
  }

  if (cache == 0 || fbo_progid == 0) {
    fbo_progid = glCreateProgram();

    glAttachShader( fbo_progid, fbo_shader );
    glAttachShader( fbo_progid, get_vertex_shader() );
    glLinkProgram( fbo_progid );

    int infolength;
    char infolog[2048];
    glGetProgramInfoLog( fbo_progid, 2048, &infolength, infolog );

    if (infolength > 0) {
      log_err("[shader:passthru:link] %s\n", infolog);
    }
  }

  glUseProgram( fbo_progid );
  glUniform1i( glGetUniformLocation( fbo_progid, "tex"), 0 );
  glUniform2fv( glGetUniformLocation( fbo_progid, "res"), 1, res );

  glActiveTexture(GL_TEXTURE0);
  //  glBindSampler(0, sampler);
  glBindTexture(GL_TEXTURE_2D, texfbo[even]);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  */
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

void layers_redraw_scribble() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void layers_cleanup() {
  layer* p = showing;

  while (p != NULL) {
    if ((p->when + (p->duration/p->cps)) < now) {
      queue_remove(&showing, p);
      if (p->is_text == 1) {
        glDeleteTextures(1, &p->textid);
      }
    }
    p = p->next;
  }
}


void layers_init() {
  log_info("[shaders] init (cache: %d)\n", cache);
  fbo = calloc(2, sizeof(GLuint));
  texfbo = calloc(2, sizeof(GLuint));
  pthread_mutex_init(&layerlock, NULL);
  pthread_mutex_init(&queuelock, NULL);

  textlayer_load_shaders();
  shaderlayer_init_noise();
  /* glGenSamplers(1, &sampler); */
  /* glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR); */
  /* glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR); */

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texfbo[0]);
  glBindTexture(GL_TEXTURE_2D, texfbo[0]);
  /* glBindSampler(0, sampler); */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res[0], res[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &texfbo[1]);
  glBindTexture(GL_TEXTURE_2D, texfbo[1]);
  /* glBindSampler(0, sampler); */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res[0], res[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);

  glGenFramebuffers(1, &fbo[0]);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, draw_buffer, GL_TEXTURE_2D, texfbo[0], 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glGenFramebuffers(1, &fbo[1]);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, draw_buffer, GL_TEXTURE_2D, texfbo[1], 0);
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawBuffers(1, &draw_buffer);
}
