#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "layer.h"
#include "shader.h"
#include "text.h"

const GLenum draw_buffer = GL_COLOR_ATTACHMENT0;

GLuint* texfbo = 0;
static GLuint vertex_shader = 0;

extern double now;
extern GLuint* fbo;

extern pthread_mutex_t layerlock;
extern pthread_mutex_t queuelock;

extern layer layers[MAXSHADERLAYERS];
extern int cache;
extern int shader_lvl;
extern int scribble;

extern layer* waiting;
extern layer* showing;

extern GLuint text_shader;
extern GLuint text_vshader;

static void layer_reset(layer* s) {
  memset(s, 0, sizeof(layer));

  s->state = UNINITIALIZED;
  s->progid = 0;
  s->shaderid = 0;
  s->next = NULL;
  s->prev = NULL;
}

void map_show_args(layer* l) {
  uarg(l, "cps", l->cps);
  uarg(l, "dur", l->duration);
  uarg4(l, "color", 1, l->color);
  uarg4(l, "position", 1, l->pos);
  uarg(l, "scale", l->scale);
  uarg(l, "speed", l->speed);
}

GLuint get_vertex_shader() {
  if (vertex_shader == 0) {
    char* filename = calloc(20, sizeof(char));
    sprintf(filename, "shaders/basic-%dxx.vert", shader_lvl);

    vertex_shader = _shader_load(filename , GL_VERTEX_SHADER );

    free(filename);
    filename = NULL;
  }
  return vertex_shader;
}

size_t read_file (FILE* file, char** content) {
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *content = malloc(fsize + 1);
  fread(*content, fsize, 1, file);

  (*content)[fsize] = 0;

  return fsize;
}

GLint _shader_load( const char *filename, GLenum type ) {
  int infolength;
  char infolog[2048];
  char* content;

  GLint shader = glCreateShader(type);

  FILE* file = fopen(filename, "r");

  if( file == NULL ) {
    log_err("[shader:load] %s could not be opend\n", filename);
    exit(1);
  }

  long size = read_file( file, &content );

  fclose(file);

  const char *const *cContent = (const char * const*) (&content);

  glShaderSource( shader, 1, cContent, ((const int*)&size) );
  glCompileShader( shader );

  glGetShaderInfoLog( shader, 2048, &infolength, infolog );

  if (infolength > 0) {
    log_err("[shader:compile] %s:\n%s\n", filename, infolog);
  }

  free (content);
  content = NULL;

  return shader;
}



void layer_copy_program(layer *cached, layer *uncached) {
  uncached->progid = cached->progid;
  uncached->shaderid = cached->shaderid;
  debug("[cache:hit]\n");
}



void layer_from_cache(layer *l) {
  if (l->is_text == 0) {
    for (int i = 0; i < MAXSHADERLAYERS; i++) {
      // TODO: implement a standard way to load _any_ layer from cache, maybe this needs a type flag in layer_T
      if (layers[i].state != UNUSED && layers[i].type_flag == l->type_flag) // layers[i].is_text == 0 && strcmp(l->shader->filename, layers[i].shader->filename) == 0)
      {
        l->f_read_cache(&layers[i], l);
        break;
      }
    }
  }
}


layer *layer_new() {
  layer *result = NULL;
  pthread_mutex_lock(&layerlock);
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (layers[i].state == UNUSED) {
      result = &layers[i];
      layer_reset(&layers[i]);
      break;
    }
  }
  pthread_mutex_unlock(&layerlock);
  debug("[shader:create]\n");
  return(result);
}


void layer_init(layer* l, t_showargs *args) {
  if (cache == 1) {
    layer_from_cache(l);
  }

  l->cps = args->cps;
  l->duration = args->dur;
  l->when = args->when;
  l->color[0] = args->r;
  l->color[1] = args->g;
  l->color[2] = args->b;
  l->color[3] = args->a;
  l->pos[0] = args->x;
  l->pos[1] = args->y;
  l->pos[2] = args->z;
  l->pos[3] = args->w;
  l->scale = args->scale;
  l->speed = args->speed;
  l->blendmode = args->blendmode;
  l->level = args->level;

  l->fontsize = args->fontsize;
}

void layer_add(layer *l) {
  pthread_mutex_lock(&queuelock);
  debug("[shader:add]\n");
  queue_add(&waiting, l);
  pthread_mutex_unlock(&queuelock);
}

void layer_blend(layer *l) {
  GLint blendmode = GL_ONE_MINUS_SRC_ALPHA;
  switch(l->blendmode) {
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
}

void layer_apply(layer *l, int even) {
  if ( l->state == UNINITIALIZED) {
    /* l->f_init(l); */
    if (l->is_text == 1) {
       textlayer_init(l);
    }

    shaderlayer_init(l);

    l->state = INITIALIZED;
  }
  if ( l->state == INITIALIZED) {
    if (l->when <= now) {
      layer_blend(l);


      /* if (scribble && l->is_scribble == 1) { */
      /*   glActiveTexture(GL_TEXTURE0); */
      /*   glBindFramebuffer(GL_FRAMEBUFFER, fbo[even]); */
      /*   glFramebufferTexture2D(GL_FRAMEBUFFER, draw_buffer, GL_TEXTURE_2D, texfbo[even], 0); */
      /* } */


      /* l->f_apply(l); */

      /*   glBindFramebuffer(GL_FRAMEBUFFER, 0); */
      /*   textlayer_finish(l); */
      /* } */
      /* else { */
      /*   if (scribble == 1) { */
      shaderlayer_apply(l);
      if (l->is_text == 1) {
        textlayer_apply(l);
      }

      /*     glBindFramebuffer(GL_FRAMEBUFFER, 0); */
      /*     shaderlayer_finish(l); */
      /*   } */
      /* } */
    }
  }
}
