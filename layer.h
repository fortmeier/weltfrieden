#ifndef _WF_LAYER_H
#define _WF_LAYER_H

#include "config.h"
#include "dbg.h"

#include "gl_env.h"
#include "t_shader.h"
#include "t_text.h"

#define uarg(s, key, value) glUniform1f( glGetUniformLocation(s->progid, key), value )
#define uarg4(s, key, num, value) glUniform4fv( glGetUniformLocation(s->progid, key), num, value)
#define uarg2fv(s, key, num, value) glUniform2fv( glGetUniformLocation(s->progid, key), num, value )

enum layerstate {UNUSED, UNINITIALIZED, INITIALIZED};

enum blendmode {
  NSA, // 1 - src alpha
  NSC, // 1 - src color
  SC, // src color
  SA, // src alpha
  SS, // src saturate
  NDA, // 1 - dst alpha
  NDC, // 1 - dst color
  DC, // dst color
  DA, // dst alpha
  CA, // const alpha
  CC // const color
};

typedef struct {
  double when;
  float cps;
  float dur;
  char *words;

  float r;
  float g;
  float b;
  float a;

  float x;
  float y;
  float z;
  float w;

  float scale;
  float speed;

  int blendmode;
  int depth;
} t_showargs;

typedef struct layer_t
{
  enum layerstate state;

  double when;
  float cps;
  float duration;

  float color[4];
  float pos[4];
  /* float rot[4]; quaternions?! */

  float scale;
  float speed;

  int is_text;
  enum blendmode blendmode;
  int depth;
  union {
    shaderlayer *shader;
    textlayer *text;
  };

  unsigned int progid;
  unsigned int shaderid;

  GLuint vao;
  GLuint vbo;

  struct layer_t *next, *prev;
} layer;

#include "queue.h"

struct layer_t* layer_new();

void layer_init(layer *l, t_showargs *args);
void layer_add(layer *l);
void layer_apply(layer *l, int even);

GLint _shader_load(const char* filename, GLenum type);

GLuint get_vertex_shader();
#endif
