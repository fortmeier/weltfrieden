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
GLuint vertex_shader = 0;

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
  uarg3(l, "rotation", 1, l->rot);
  uarg3(l, "origin", 1, l->origin);

  uarg(l, "width", l->width);
  uarg(l, "height", l->height);
  uarg(l, "speed", l->speed);
}

GLuint get_vertex_shader() {
  if (vertex_shader == 0) {
    char filename[256];
    sprintf(filename, "shaders/basic-%dxx.vert", shader_lvl);

    vertex_shader = _shader_load(filename , GL_VERTEX_SHADER );

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
  pthread_mutex_lock(&layerlock);
  uncached->progid = cached->progid;
  uncached->shaderid = cached->shaderid;
  uncached->textid = cached->textid;
  uncached->is_image = cached->is_image;
  pthread_mutex_unlock(&layerlock);
  debug("[cache:hit]");
}



void layer_from_cache(layer *l) {
  int cached = 0;
  int tries = 0;
  if (l->is_text == 0) {
    for (int i = 0; i < MAXSHADERLAYERS; i++) {
      // TODO: implement a standard way to load _any_ layer from cache, maybe this needs a type flag in layer_T
      if ((layers[i].state == INITIALIZED || layers[i].state == SHOWN) && layers[i].type_flag == l->type_flag) // layers[i].is_text == 0 && strcmp(l->shader->filename, layers[i].shader->filename) == 0)
      {
        cached = l->f_read_cache(&layers[i], l);
        if (cached == 1) {
          break;
        }
        tries++;
      }
    }
    if (cached == 1) {
      debug("[layer:cache:hit] HIT after %d tries", tries);
    }
    else {
      debug("[layer:cache:miss] MISS after %d tries", tries);
    }

  }
  else {
    debug("[layer:cache:text] cannot cache text");
  }
}


layer *layer_new() {
  layer *result = NULL;
  pthread_mutex_lock(&layerlock);
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (layers[i].state == UNUSED || layers[i].state == SHOWN) {
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

  l->rot[0] = args->rot_x;
  l->rot[1] = args->rot_y;
  l->rot[2] = args->rot_z;

  l->origin[0] = args->origin_x;
  l->origin[1] = args->origin_y;
  l->origin[2] = args->origin_z;

  l->width = args->width;
  l->height = args->height;
  l->speed = args->speed;
  l->srcblend = args->srcblend;
  l->dstblend = args->dstblend;
  l->blendeq = args->blendeq;
  l->level = args->level;

  l->fontsize = args->fontsize;
  l->charcode = args->charcode;
}

void layer_add(layer *l) {
  pthread_mutex_lock(&queuelock);
  debug("[shader:add]\n");
  queue_add(&waiting, l);
  pthread_mutex_unlock(&queuelock);
}

void layer_blend(layer *l) {
  GLint srcblend = GL_SRC_ALPHA;
  GLint dstblend = GL_ONE_MINUS_SRC_ALPHA;
  GLint blendeq = GL_FUNC_ADD;

  switch(l->srcblend) {
  case NSA: srcblend = GL_ONE_MINUS_SRC_ALPHA; break;
  case NSC: srcblend = GL_ONE_MINUS_SRC_COLOR; break;
  case NDA: srcblend = GL_ONE_MINUS_DST_ALPHA; break;
  case NDC: srcblend = GL_ONE_MINUS_DST_COLOR; break;
  case SA: srcblend = GL_SRC_ALPHA; break;
  case SC: srcblend = GL_SRC_COLOR; break;
  case DA: srcblend = GL_DST_ALPHA; break;
  case DC: srcblend = GL_DST_COLOR; break;
  case SS: srcblend = GL_SRC_ALPHA_SATURATE; break;
  case CC: srcblend = GL_CONSTANT_COLOR; break;
  case CA: srcblend = GL_CONSTANT_ALPHA; break;
  }

  switch(l->dstblend) {
  case NSA: dstblend = GL_ONE_MINUS_SRC_ALPHA; break;
  case NSC: dstblend = GL_ONE_MINUS_SRC_COLOR; break;
  case NDA: dstblend = GL_ONE_MINUS_DST_ALPHA; break;
  case NDC: dstblend = GL_ONE_MINUS_DST_COLOR; break;
  case SA: dstblend = GL_SRC_ALPHA; break;
  case SC: dstblend = GL_SRC_COLOR; break;
  case DA: dstblend = GL_DST_ALPHA; break;
  case DC: dstblend = GL_DST_COLOR; break;
  case SS: dstblend = GL_SRC_ALPHA_SATURATE; break;
  case CC: dstblend = GL_CONSTANT_COLOR; break;
  case CA: dstblend = GL_CONSTANT_ALPHA; break;
  }

  switch(l->blendeq) {
  case FA: blendeq = GL_FUNC_ADD; break;
  case FS: blendeq = GL_FUNC_SUBTRACT; break;
  case FRS: blendeq = GL_FUNC_REVERSE_SUBTRACT; break;
  case FMIN: blendeq = GL_MIN; break;
  case FMAX: blendeq = GL_MAX; break;
  }

  glBlendEquation(blendeq);
  glBlendFunc(srcblend, dstblend);
}

void layer_apply(layer *l, int even) {
  if ( l->state == UNINITIALIZED) {
    /* l->f_init(l); */
    if (l->is_text == 1) {
       textlayer_init(l);
    }

    if (l->progid == 0) {
      shaderlayer_init(l);
    }

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
      pthread_mutex_lock(&layerlock);
      shaderlayer_apply(l);
      if (l->is_text == 1) {
        textlayer_apply(l);
      }
      pthread_mutex_unlock(&layerlock);

      /*     glBindFramebuffer(GL_FRAMEBUFFER, 0); */
      /*     shaderlayer_finish(l); */
      /*   } */
      /* } */
    }
  }
}
