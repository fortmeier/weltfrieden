#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <pthread.h>
#include <math.h>

#ifdef EGL_RPI2
#else
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "text.h"

#define TEXT_SIZE_F 10000

typedef struct t_character {
  GLuint texid;
  GLuint res[2];
  GLuint bearing[2];
  GLuint advance;
} character;

#define MAXCHARS 512

character characters[MAXCHARS];

character t_char;

extern float res[2];
extern GLuint* texfbo;
extern GLuint sampler;
extern double now;
extern GLuint vao;
extern int cache;
extern int shader_lvl;

typedef struct {
  float x, y, z;    // position
  float s, t;       // texture
  float r, g, b, a; // color
} vertex_t;

GLuint text_vshader = 0;
GLuint text_shader = 0;



void textlayer_init(layer* l) {
  /* textlayer *t = (textlayer*)l->layer_data; */
  #ifdef EGL_RPI2
  #else
  FT_Library ft;

  if (FT_Init_FreeType(&ft)) {
    log_err("[text:init] freetype could not be initialized\n");
    exit(1);
  }

  FT_Face face;
  if (FT_New_Face(ft, "fonts/VeraMono.ttf", 0, &face)) {
    log_err("[text:init] could not be loaded\n");
    exit(1);
  }

  float fontsize = fmin(l->fontsize, 40) * TEXT_SIZE_F / res[1];

  FT_Set_Pixel_Sizes(face, 0, fontsize);

  if (FT_Load_Char(face, l->text[0], FT_LOAD_RENDER)) {
    log_err("[text:init] failed loading char\n");
    exit(1);
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 8-bit gray scale bitmap coming up!

  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_LUMINANCE,
    face->glyph->bitmap.width,
    face->glyph->bitmap.rows,
    0,
    GL_LUMINANCE,
    GL_UNSIGNED_BYTE,
    face->glyph->bitmap.buffer
    );

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  l->textid = texture;
  GLuint text_res[2] = {
    face->glyph->bitmap.width,
    face->glyph->bitmap.rows
  };

  GLuint text_bearing[2] = {
    face->glyph->bitmap_left,
    face->glyph->bitmap_top
  };

  debug("[text:params] %d,%d %d,%d %f\n", text_res[0], text_res[1], text_bearing[0], text_bearing[1], fontsize);

  GLuint text_advance = face->glyph->advance.x;

  GLfloat w = text_res[0] / res[0];
  GLfloat h = text_res[1] / res[0];

  GLfloat xpos = ( (text_bearing[0]) / fontsize ) + l->pos[0] - (w/2);
  GLfloat ypos = ((l->pos[1] - (text_res[1] - text_bearing[1])) / fontsize) + (l->pos[1]) - (h/2);

  GLfloat vertices[6][4] = {
    { xpos, ypos + h, 0.0, 0.0 },
    { xpos, ypos, 0.0, 1.0 },
    { xpos + w, ypos, 1.0, 1.0 },
    { xpos, ypos + h, 0.0, 0.0 },
    { xpos + w, ypos, 1.0, 1.0 },
    { xpos + w, ypos + h, 1.0, 0.0 }
  };

  log_info("[text:res] %f,%f %f,%f", xpos, ypos, w, h);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  
  glGenBuffers(1, &l->text_vbo);
  
  #ifndef EGL_RPI2
  glGenVertexArrays(1, &l->text_vao);
  glBindVertexArray(l->text_vao);
  #endif
  glBindBuffer(GL_ARRAY_BUFFER, l->text_vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, vertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  #ifndef EGL_RPI2
  glBindVertexArray(0);
  #endif
  glBindTexture(GL_TEXTURE_2D, 0);
  l->text_progid = glCreateProgram();
  glAttachShader(l->text_progid, text_vshader);
  glAttachShader(l->text_progid, text_shader);
  glLinkProgram(l->text_progid);
  #endif
}

void textlayer_add(t_showargs args) {
  layer* l = textlayer_new();
  textlayer* t = (textlayer*)l->layer_data;

  if (l == NULL) {
    log_err("[text:new] hit max (%d)\n", MAXSHADERLAYERS);
    return;
  }

  t->text = strdup(args.words);

  layer_init(l, &args);
  layer_add(l);
}


void textlayer_load_shaders() {
  char filename[256];

  sprintf(filename, "shaders/txt-%dxx.vert",
          shader_lvl);

  text_vshader = _shader_load( filename, GL_VERTEX_SHADER);
  
  sprintf(filename, "shaders/txt-%dxx.frag",
          shader_lvl);

  text_shader = _shader_load( filename, GL_FRAGMENT_SHADER);

}

void textlayer_apply(layer* l) {
  if (cache == 0) {
    textlayer_load_shaders();
  }

  glUseProgram(l->text_progid);
  glUniform3f( glGetUniformLocation(l->text_progid, "text_color"), l->color[0], l->color[1], l->color[2] );
  glUniform2fv( glGetUniformLocation(l->text_progid, "res"), 1, res);
  uarg(l, "elapsed", now - l->when);
  uarg(l, "cps", l->cps);
  uarg(l, "dur", l->duration);

  /* glBindSampler(1, sampler); */
  /* glUniform1i( glGetUniformLocation(l->text_progid, "fbotex"), 1); */
  /* glActiveTexture(GL_TEXTURE1); */
  /* glBindTexture(GL_TEXTURE_2D, texfbo[even]); */

  glActiveTexture(GL_TEXTURE0);
  #ifndef EGL_RPI2
  glBindVertexArray(l->text_vao);
  #endif
  glBindTexture(GL_TEXTURE_2D, l->textid);

  glDrawArrays(GL_TRIANGLES, 0, 6);
  #ifndef EGL_RPI2
  glBindVertexArray(0);
  #endif
  glBindTexture(GL_TEXTURE_2D, 0);
}

void textlayer_finish(layer* l) {
}



void textlayer_read_cache(layer *cached, layer *uncached) {
  textlayer *s_uncached = (textlayer*)uncached->layer_data;
  textlayer *s_cached = (textlayer*)cached->layer_data;

  // this could be better, since text is not really cached up to now, just reuse the shader program
  if (strcmp(s_cached->text, s_uncached->text) != -1) {
    layer_copy_program(cached, uncached);
  }
}


layer *textlayer_new() {
  layer *l = layer_new();
  if (l == NULL) {
    return(NULL);
  }

  l->layer_data = (void*)malloc(sizeof(textlayer));
  l->is_text = 1;
  l->is_scribble = 0;
  l->type_flag = TEXTLAYER_TYPE_FLAG;
  l->f_apply = textlayer_apply;
  l->f_init = textlayer_init;
  l->f_read_cache = textlayer_read_cache;
  return(l);
}
