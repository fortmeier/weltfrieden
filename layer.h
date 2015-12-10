#ifndef _WF_LAYER_H
#define _WF_LAYER_H

#include "config.h"
#include "dbg.h"

#include "gl_env.h"

#define uarg(s, key, value) glUniform1f( glGetUniformLocation(s->progid, key), value )
#define uarg4(s, key, num, value) glUniform4fv( glGetUniformLocation(s->progid, key), num, value)
#define uarg3(s, key, num, value) glUniform3fv( glGetUniformLocation(s->progid, key), num, value)

#define uarg2fv(s, key, num, value) glUniform2fv( glGetUniformLocation(s->progid, key), num, value )

enum layerstate {UNUSED, UNINITIALIZED, LOADING, INITIALIZED, SHOWN};

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

enum blendeq {
  FA, // FUNC_ADD
  FS, // FUNC_SUBTRACT
  FRS, // FUNC_REVERSE_SUBTRACT
  FMIN, // MIN
  FMAX // MAX
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

  float rot_x;
  float rot_y;
  float rot_z;

  float origin_x;
  float origin_y;
  float origin_z;

  float width;
  float height;
  float speed;

  int srcblend;
  int dstblend;
  int blendeq;
  int level;
} t_showargs;

struct layer_t;

typedef void (*f_layer_apply)(struct layer_t *l);
typedef void (*f_layer_init)(struct layer_t *l);
typedef int (*f_layer_read_cache)(struct layer_t *cached, struct layer_t *uncached);


typedef struct layer_t
{
  enum layerstate state;

  int type_flag;

  double when;
  float cps;
  float duration;

  float color[4];
  float pos[4];
  float rot[3]; /* quaternions?! */
  float origin[3];

  float width;
  float height;
  float speed;


  int is_image;

  enum blendmode srcblend;
  enum blendmode dstblend;
  enum blendeq blendeq;
  int level;

  char *filename;
  char *filecontent;

  /* void *layer_data; */

  /* f_layer_apply f_apply; */
  /* f_layer_init f_init; */
  /* f_layer_read_cache f_read_cache; */

  unsigned int progid;
  unsigned int shaderid;
  unsigned int textid;

  GLuint vao;
  GLuint vbo;

  struct layer_t *next, *prev;
} layer;

#include "queue.h"




struct layer_t* layer_new();

void map_show_args(layer *l);

void layer_init(layer *l, t_showargs *args);
void layer_add(t_showargs args, int is_image, int textid);
void layer_apply(layer *l, int even);

void layer_copy_program(layer *cached, layer *uncached);

GLint _shader_load(const char* filename, GLenum type);

GLuint get_vertex_shader();

#endif
