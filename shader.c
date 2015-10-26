#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "shader.h"


extern GLuint *texfbo;
extern GLuint sampler;
extern GLuint vao;
extern GLuint vbo;
extern int cache;

int scribble = 0;
float cursor[2];
double now = 0;
float res[2];
int shader_lvl = 0;

layer *shaderlayer_new() {
  layer *l = layer_new();
  if (l == NULL) {
    return(NULL);
  }

  l->layer_data = (void*)malloc(sizeof(shaderlayer));
  l->is_text = 0;
  l->type_flag = SHADERLAYER_TYPE_FLAG;
  l->f_apply = shaderlayer_apply;
  l->f_init = shaderlayer_init;
  l->f_read_cache = shaderlayer_read_cache;

  return(l);
}

void shaderlayer_read_cache(layer *cached, layer *uncached) {
  shaderlayer *s_uncached = (shaderlayer*)uncached->layer_data;
  shaderlayer *s_cached = (shaderlayer*)cached->layer_data;

  if (strcmp(s_cached->filename, s_uncached->filename) != -1) {
    layer_copy_program(cached, uncached);
  }
}

void shaderlayer_init(layer* l) {
  /* debug("[shader:cache:miss] %s\n", l->shader->filename); */
  l->progid = glCreateProgram();

  char filename[256];

  shaderlayer *s = (shaderlayer*)l->layer_data;

  sprintf(filename, "shaders/%s-%dxx.frag",
          s->filename,
          shader_lvl);

  l->shaderid = _shader_load( filename, GL_FRAGMENT_SHADER );
  glAttachShader( l->progid, l->shaderid );

  glAttachShader( l->progid, get_vertex_shader() );

  glLinkProgram( l->progid );

  int infolength;
  char infolog[2048];
  glGetProgramInfoLog( l->progid, 2048, &infolength, infolog );

  if (infolength > 0) {
    log_err("[shader:link] %s\n%s\n", filename, infolog);
  }

}


void shaderlayer_add(t_showargs args) {
  layer* l = shaderlayer_new();
  if (l == NULL) {
    log_err("[shader:new] hit max (%d)\n", MAXSHADERLAYERS);
    return;
  }

  shaderlayer *s = (shaderlayer*)l->layer_data;

  s->filename = strdup(args.words);

  if (strcmp(s->filename, "scribble") != -1) {
    l->is_scribble = 1;
  }

  l->is_text = 0;

  layer_init(l, &args);
  layer_add(l);
}


void shaderlayer_apply(layer *l) {
  /* glUniform1i( glGetUniformLocation(l->progid, "fbotex"), 0); */
  /* glActiveTexture(GL_TEXTURE1); */
  /* glBindTexture(GL_TEXTURE_2D, texfbo[even]); */
  /* glBindSampler(0, sampler); */

  uarg(l, "now", now);
  uarg(l, "elapsed", now - l->when);
  uarg2fv(l, "res", 1, res);
  uarg2fv(l, "cursor", 1, cursor);

  map_show_args(l);

  if (shader_lvl >= 3) {
    glBindVertexArray(vao);
  }
  else {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  }
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

void shaderlayer_finish(layer *l) {
}
