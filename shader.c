#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "shader.h"

#define uarg(s, key, value) glUniform1f( glGetUniformLocation(s->progid, key), value )
#define uarg4(s, key, num, value) glUniform4fv( glGetUniformLocation(s->progid, key), num, value)
#define uarg2fv(s, key, num, value) glUniform2fv( glGetUniformLocation(s->progid, key), num, value )


extern GLuint *texfbo;
extern GLuint sampler;
extern GLuint vao;
extern GLuint vbo;
extern int cache;

double now = 0;
float res[2];
int shader_lvl = 0;

void map_show_args(layer* l) {
  uarg(l, "cps", l->cps);
  uarg(l, "dur", l->duration);
  uarg4(l, "color", 1, l->color);
  uarg4(l, "position", 1, l->pos);
  uarg(l, "scale", l->scale);
  uarg(l, "speed", l->speed);
}

layer *shaderlayer_new() {
  layer *l = layer_new();
  if (l == NULL) {
    return(NULL);
  }

  l->shader = malloc(sizeof(shaderlayer));
  l->is_text = 0;

  return(l);
}

void shaderlayer_init(layer* l) {
  debug("[shader:cache:miss] %s\n", l->shader->filename);
  l->progid = glCreateProgram();

  char filename[256];

  sprintf(filename, "shaders/%s-%dxx.frag",
          l->shader->filename,
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

  l->shader->filename = strdup(args.words);

  l->is_text = 0;

  layer_init(l, &args);
  layer_add(l);
}


void shaderlayer_apply(layer *l, int even) {
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


  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texfbo[even]);
  glBindSampler(0, sampler);

  uarg(l, "now", now);
  uarg(l, "elapsed", now - l->when);
  uarg2fv(l, "res", 1, res);

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
  printf("s");
}

void shaderlayer_finish(layer *l) {
}
