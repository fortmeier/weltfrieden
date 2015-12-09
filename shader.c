#include <assert.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "shader.h"

#include "stb_image.h"
#include "stb_image_resize.h"

#define MAXPATHSIZE 256

extern GLuint *texfbo;
extern GLuint sampler;
extern GLuint vao;
extern GLuint vbo;

// options
extern int cache;
extern char* imageroot;

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


int jpg_filter(const struct dirent *d) {
  if (strlen(d->d_name) > 4) {
    return(strcmp(d->d_name + strlen(d->d_name) - 4, ".jpg") == 0 ||
           strcmp(d->d_name + strlen(d->d_name) - 4, ".JPG") == 0);
  }
  return(0);
}

void shaderlayer_init(layer* l) {
  l->progid = glCreateProgram();

  char filename[256];

  shaderlayer *s = (shaderlayer*)l->layer_data;
  debug("[shader:cache:miss] %s\n", s->filename);

  if ( (strcmp(s->filename, "plane") == 0) || (strcmp(s->filename, "tri") == 0) || (strcmp(s->filename, "circle") == 0) ) {

    sprintf(filename, "shaders/%s-%dxx.frag",
            s->filename,
            shader_lvl);
    l->is_image = 0;

  }
  else {
    // filename is an image filename
    sprintf(filename, "shaders/image-%dxx.frag",
            shader_lvl);
    l->is_image = 1;

    // load an image
    int iw,ih,in,set_n;
    char set[MAXPATHSIZE];
    char sep[2];
    char path[MAXPATHSIZE];
    struct dirent **namelist;

    if (sscanf(s->filename, "%[a-z0-9A-Z]%[/:]%d", set, sep, &set_n)) {
      int n;
      snprintf(path, MAXPATHSIZE -1, "%s/%s", imageroot, set);
      //printf("looking in %s\n", set);
      n = scandir(path, &namelist, jpg_filter, alphasort);
      if (n > 0) {
        snprintf(path, MAXPATHSIZE -1,
                 "%s/%s/%s", imageroot, set, namelist[set_n % n]->d_name);
        while (n--) {
          free(namelist[n]);
        }
        free(namelist);
      } else {
        snprintf(path, MAXPATHSIZE -1, "%s/%s", imageroot, s->filename);
      }
    } else {
      snprintf(path, MAXPATHSIZE -1, "%s/%s", imageroot, s->filename);
    }


    unsigned char* raw_image = stbi_load(path, &iw, &ih, &in, 0);
    unsigned char* image = (unsigned char*) malloc(res[0]*res[1]*in);
    if (raw_image) {
      log_info("raw image loaded: %d %d (%d)", iw, ih, in);

      /* fit image ratio into resolution ratio */
      float s0 = 0,
        t0 = 0,
        s1 = (iw > ih) ? ((float)ih/(float)iw) : 1,
        t1 = (iw > ih) ? 1 : ((float)iw/(float)ih);

      int resize_result = stbir_resize_region(raw_image, iw, ih, 0,
                                              image, res[0], res[1], 0,
                                              STBIR_TYPE_UINT8, in, STBIR_ALPHA_CHANNEL_NONE, 0,
                                              STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_BOX,
                                              STBIR_FILTER_CATMULLROM, STBIR_COLORSPACE_SRGB, NULL,
                                              s0, t0, s1, t1);



      if (resize_result == 1) {
        GLuint texture;
        log_info("[load:image] %fx%f (%f, %f)", res[0], res[1], s1, t1);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGB,
          res[0],
          res[1],
          0,
          GL_RGB,
          GL_UNSIGNED_BYTE,
          image
          );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        l->textid = texture;
        debug("[image:tex:create] %d", l->textid);

        stbi_image_free(image);
      }
      else {
        log_err("cannot resize image");
      }

      stbi_image_free(raw_image);

    }
    else {
      log_err("cannot load image");
    }

  }
  log_info("loading frag shader %s", filename);
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
  /* glBindSampler(0, sampler); */
  glUseProgram(l->progid);
  if (l->is_image == 1) {
    glUniform1i( glGetUniformLocation( l->progid, "tex"), 0 );
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, l->textid);
  }

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
