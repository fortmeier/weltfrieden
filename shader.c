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
float offset[2];
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

int shaderlayer_read_cache(layer *cached, layer *uncached) {
  shaderlayer *s_uncached = (shaderlayer*)uncached->layer_data;
  shaderlayer *s_cached = (shaderlayer*)cached->layer_data;

  if (strcmp(s_cached->filename, s_uncached->filename) == 0) {
    layer_copy_program(cached, uncached);
    debug("LAYER PROGRAM COPIED %d -> %d for %s", cached->progid, uncached->progid, s_uncached->filename);
    return 1;
  }
  else {
    return 0;
  }
}

void shaderlayer_init_noise() {
  /* if (shader_noise_id == 0) { */
    shader_noise_id = glCreateProgram();

    char filename[256];

    sprintf(filename, "shaders/%s-%dxx.frag",
            shader_noise_name,
            shader_lvl);

    GLuint shaderid;
    shaderid = _shader_load( filename, GL_FRAGMENT_SHADER );
    glAttachShader( shader_noise_id, shaderid );


    glAttachShader( shader_noise_id, get_vertex_shader() );

    glLinkProgram( shader_noise_id );

    int infolength;
    char infolog[2048];
    glGetProgramInfoLog( shader_noise_id, 2048, &infolength, infolog );

    if (infolength > 0) {
      log_err("[shader:link] %s\n%s\n", filename, infolog);
    }
    debug("[shader:noise:init]");
  /* } */

}

void shaderlayer_init(layer* l) {
  l->progid = glCreateProgram();

  char filename[256];

  shaderlayer *s = (shaderlayer*)l->layer_data;
  debug("[shader:cache:miss] %s\n", s->filename);

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

//  shaderlayer_init_noise();
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

  if (strlen(args.text) > 0 || args.charcode != -1) {
    l->is_text = 1;
    l->text = strdup(args.text);
    debug("[text:add] %s %f\n", l->text, args.fontsize);
  }

  layer_init(l, &args);
  layer_add(l);
}


void shaderlayer_apply(layer *l) {
  /* glUniform1i( glGetUniformLocation(l->progid, "fbotex"), 0); */
  /* glActiveTexture(GL_TEXTURE1); */
  /* glBindTexture(GL_TEXTURE_2D, texfbo[even]); */
  /* glBindSampler(0, sampler); */
  glUseProgram(l->progid);

  uarg(l, "now", now);
  uarg(l, "elapsed", now - l->when);
  uarg2fv(l, "res", 1, res);
  uarg2fv(l, "offset", 1, offset);
  uarg2fv(l, "cursor", 1, cursor);

  map_show_args(l);

  if (shader_lvl >= 3) {
    glBindVertexArray(vao);
  }
  else {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float) ,(void *) ( 0 * sizeof(float) ));
  }
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

  /* glUseProgram(shader_noise_id); */

  /* uarg(l, "now", now); */
  /* uarg(l, "elapsed", now - l->when); */
  /* uarg2fv(l, "res", 1, res); */
  /* uarg2fv(l, "cursor", 1, cursor); */

  /* map_show_args(l); */

  /* if (shader_lvl >= 3) { */
  /*   glBindVertexArray(vao); */
  /* } */
  /* else { */
  /*   glEnableVertexAttribArray(0); */
  /*   glBindBuffer(GL_ARRAY_BUFFER, vbo); */
  /*   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float) ,(void *) ( 0 * sizeof(float) )); */
  /* } */

  /* glDrawArrays (GL_TRIANGLE_STRIP, 0, 4); */
  glUseProgram(0);
}

void shaderlayer_finish(layer *l) {
}
