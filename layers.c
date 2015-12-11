#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

#include "layers.h"
#include "thpool.h"
#include "stb_image.h"
#include "stb_image_resize.h"

#define MAXSHADERLAYERS 512
#define MAXIMAGES 1024
#define MAXPATHSIZE 256

GLuint sampler = 0;
GLuint* fbo = 0;

double now = 0;
float res[2];
float offset[2];
int shader_lvl = 0;

layer* waiting = NULL;
layer* showing = NULL;

static int pass = 0;

layer layers[MAXSHADERLAYERS];
image_t* images[MAXIMAGES];

int image_count = 0;

pthread_mutex_t layerlock;
pthread_mutex_t queuelock;
pthread_mutex_t imagelock;
pthread_mutex_t imageloadinglock;

thpool_t* read_file_pool;

typedef struct read_file_args {
  t_showargs* args;
  char *filename;
} read_file_args_t;

char **images_loading;

// options
extern int cache;
extern char* imageroot;

extern GLFWwindow* win;

GLuint vbo;
GLuint vao;

GLuint vertex_shader = 0;

static bool is_image_loading(const char* imagename);
static void mark_as_loading(const char* imagename);
static void unmark_as_loading(const char* imagename);
static t_showargs* copy_show_args(const t_showargs* orig_args);

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
#ifndef NDEBUG
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
#ifndef NDEBUG
    assert(s == (queue_size(showing) - 1));
#endif
  }
  pthread_mutex_unlock(&queuelock);
}

image_t* find_image(char *imagename) {
  image_t *img = NULL;

  for (int c = 0; c < image_count; c++) {
    if(strcmp(images[c]->name, imagename) == 0) {
      img = images[c];
      break;
    }
  }

  return(img);
}

void shaderlayer_init(layer* l) {
  l->progid = glCreateProgram();
  char filename[256];

  debug("[shader:cache:miss] %s %d %d", l->filename, l->is_image, l->textid);

  if (l->is_image == 1) {
    sprintf(filename, "shaders/image-%dxx.frag",
            shader_lvl);
  }
  else {
    sprintf(filename, "shaders/%s-%dxx.frag",
          l->filename,
          shader_lvl);
  }

  debug("loading frag shader %s", filename);
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

void shaderlayer_apply(layer *l) {
  GLuint boundTexture = 0;

  glfwMakeContextCurrent(win);
  glUseProgram(l->progid);
  assert((l->is_image == 1) || (l->textid == 0));
  if (l->is_image == 1) {
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*) &boundTexture); // store currently bound texture id
    glUniform1i( glGetUniformLocation( l->progid, "tex"), 0 );
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, l->textid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // these are essential, otherwise texture cannot be accessed in shader
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }

  uarg(l, "now", now);
  uarg(l, "elapsed", now - l->when);
  uarg2fv(l, "res", 1, res);
  uarg2fv(l, "offset", 1, offset);

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

  // restore original state
  if (l->is_image == 1) {
    glBindTexture(GL_TEXTURE_2D, boundTexture);
  }
  glUseProgram(0);
}

void shaderlayer_finish(layer *l) {
}

void layers_destroy() {
  pthread_mutex_destroy(&layerlock);
  pthread_mutex_destroy(&queuelock);
  pthread_mutex_destroy(&imagelock);
  pthread_mutex_destroy(&imageloadinglock);

  if (read_file_pool) {
    thpool_destroy(read_file_pool);
  }
}

void layers_finish(int even) {
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

int jpg_filter(const struct dirent *d) {
  if (strlen(d->d_name) > 4) {
    return(strcmp(d->d_name + strlen(d->d_name) - 4, ".jpg") == 0 ||
           strcmp(d->d_name + strlen(d->d_name) - 4, ".JPG") == 0);
  }
  return(0);
}

GLuint image_to_texture(const char* filename) {
  // load an image
  int iw,ih,in,set_n;
  char set[MAXPATHSIZE];
  char sep[2];
  char path[MAXPATHSIZE];
  GLuint texture;
  glfwMakeContextCurrent(win);
  struct dirent **namelist;
  debug("[image:find] %s in %s", filename, imageroot);
  if (sscanf(filename, "%[a-z0-9A-Z]%[/:]%d", set, sep, &set_n)) {
    int n;
    snprintf(path, MAXPATHSIZE -1, "%s/%s", imageroot, set);

    n = scandir(path, &namelist, jpg_filter, alphasort);
    if (n > 0) {
      snprintf(path, MAXPATHSIZE -1,
               "%s/%s/%s", imageroot, set, namelist[set_n % n]->d_name);
      while (n--) {
        free(namelist[n]);
      }
      free(namelist);
    } else {
      snprintf(path, MAXPATHSIZE -1, "%s/%s", imageroot, filename);
    }
  } else {
    snprintf(path, MAXPATHSIZE -1, "%s/%s", imageroot, filename);
  }

  unsigned char* raw_image = stbi_load(path, &iw, &ih, &in, 0);
  unsigned char* image = (unsigned char*) malloc(res[0]*res[1]*in);
  if (raw_image) {
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
      log_info("[load:image] %s (%.fx%.f)", filename, res[0], res[1]);
      GLuint boundTexture = 0;
      glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*) &boundTexture);

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

      glBindTexture(GL_TEXTURE_2D, boundTexture);
      stbi_image_free(image);
    }
    else {
      return -1;
    }

    stbi_image_free(raw_image);
  }
  else {
    return -1;
  }
  debug("[image:resized] %s", filename);

  return texture;
}


void read_image_func(void* raw_args) {
  read_file_args_t* args = (read_file_args_t*)raw_args;
  char* filename = strdup(args->filename);
  image_t* img;

  img = find_image(filename);

  if (img == NULL) {

    int texture = image_to_texture(filename);

    if (texture == -1) {
      log_err("failed loading image: %s", filename);
    }
    else {
      img = (image_t *) calloc(1, sizeof(image_t));
      img->name = filename;
      img->texture = texture;
      pthread_mutex_lock(&imagelock);
      images[image_count++] = img;
      pthread_mutex_unlock(&imagelock);
    }
  }
  else {
    debug("[image:cache] HIT %s", filename);
  }

  if (img == NULL) {
    log_err("failed caching image: %s", filename);
  }
  else {
    debug("[image:loaded] %s", filename);
    layer_add(*(args->args), 1, img->texture);
  }

  unmark_as_loading(filename);

  free(args->filename);
  free(args);
}

void layers_add(t_showargs args) {

    if ( (strcmp(args.words, "plane") == 0) || (strcmp(args.words, "tri") == 0) || (strcmp(args.words, "circle") == 0) ) {
      layer_add(args, 0, 0);
    }
    else {
      // load images async
      if (!is_image_loading(args.words)) {
        mark_as_loading(args.words);
        read_file_args_t *read_args = malloc(sizeof(read_file_args_t));

        read_args->filename = strdup(args.words);
        read_args->args = copy_show_args(&args);
        if (!thpool_add_job(read_file_pool, (void*) read_image_func, (void*) read_args)) {
          log_err("[layer:apply] Could not add image reading job for '%s'", read_args->filename);
        }
      }
    }

}

void layers_cleanup() {
  layer* p = showing;

  while (p != NULL) {
    if ((p->when + (p->duration/p->cps)) < now) {
      queue_remove(&showing, p);
      p->state = SHOWN;
    }
    p = p->next;
  }
}

void layers_clear_cache() {
  pthread_mutex_lock(&layerlock);
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    int state = layers[i].state;

    switch (state) {
    case INITIALIZED:
      layers[i].state = UNINITIALIZED;
      break;
    case SHOWN:
      layers[i].state = UNUSED;
      break;
    }
  }
  pthread_mutex_unlock(&layerlock);
}

static void init_images_loading() {
  images_loading = malloc(sizeof(char*) * thpool_size(read_file_pool));
  if (!images_loading) {
    fprintf(stderr, "no memory to allocate `images_loading'\n");
    exit(1);
  }
  for (int i = 0; i < thpool_size(read_file_pool); i++) {
    images_loading[i] = NULL;
  }
}

void layers_init(int num_workers) {
  read_file_pool = thpool_init(num_workers);
  if (!read_file_pool) {
    log_err("[layers:init] cannot create `read_file_pool`");
    exit(1);
  }
  init_images_loading();
  log_info("[shaders] init (cache: %d)\n", cache);
  pthread_mutex_init(&layerlock, NULL);
  pthread_mutex_init(&queuelock, NULL);
  pthread_mutex_init(&imagelock, NULL);
  pthread_mutex_init(&imageloadinglock, NULL);
}

static void layer_reset(layer* s) {
  memset(s, 0, sizeof(layer));

  s->state = UNINITIALIZED;
  s->progid = 0;
  s->shaderid = 0;
  s->textid = 0;
  s->next = NULL;
  s->prev = NULL;
  if (s->is_image == 1) {
    debug("[texture:delete] %d", s->textid);
    glDeleteTextures(1, &(s->textid));
  }
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

GLint _shader_load( const char *filename, GLenum type) {
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
  for (int i = 0; i < MAXSHADERLAYERS; i++) {
    if (layers[i].state == INITIALIZED || layers[i].state == SHOWN)
    {
      if (strcmp(layers[i].filename, l->filename) == 0) {
        layer_copy_program(&layers[i], l);
        debug("LAYER PROGRAM COPIED %d -> %d for %s", layers[i].progid, l->progid, l->filename);

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
}

void layer_add(t_showargs args, int is_image, int textid) {
  layer* l = layer_new();
  if (l == NULL) {
    log_err("[shader:new] hit max (%d)\n", MAXSHADERLAYERS);
    return;
  }

  l->filename = strdup(args.words);

  l->is_image = is_image;
  l->textid = textid;
  layer_init(l, &args);

  pthread_mutex_lock(&queuelock);
  debug("[shader:add] %s %d %d", l->filename, l->is_image, l->textid);
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
  pthread_mutex_lock(&layerlock);
  if ( l->state == UNINITIALIZED) {
    if (l->progid == 0) { // this is a cache miss, we have to compile and link a shader program
      shaderlayer_init(l);
    }

    l->state = INITIALIZED;
  }
  if ( l->state == INITIALIZED) {
    if (l->when <= now) {
      layer_blend(l);


      shaderlayer_apply(l);

    }
  }
  pthread_mutex_unlock(&layerlock);
}

static bool is_image_loading(const char* imagename) {
  for (int i = 0; i < thpool_size(read_file_pool); i++) {
    if (images_loading[i] != NULL && strcmp(images_loading[i], imagename) == 0) {
      return true;
    }
  }
  return false;
}

static void mark_as_loading(const char* imagename) {
  pthread_mutex_lock(&imageloadinglock);

  int i;
  for (i = 0; i < thpool_size(read_file_pool); i++) {
    if (images_loading[i] == NULL) break;
  }
  images_loading[i] = strdup(imagename);

  pthread_mutex_unlock(&imageloadinglock);
}

static void unmark_as_loading(const char* imagename) {
  pthread_mutex_lock(&imageloadinglock);

  int i;
  for (i = 0; i < thpool_size(read_file_pool); i++) {
    const char* sn = images_loading[i];
    if (sn != NULL && strcmp(sn, imagename) == 0) break;
  }
  free(images_loading[i]);
  images_loading[i] = NULL;

  pthread_mutex_unlock(&imageloadinglock);
}

static t_showargs* copy_show_args(const t_showargs* orig_args) {
  t_showargs* args = malloc(sizeof(t_showargs));
  if (args == NULL) {
    fprintf(stderr, "no memory to allocate play arguments struct\n");
    return NULL;
  }
  memcpy(args, orig_args, sizeof(t_showargs));
  args->words = strdup(orig_args->words);
  return args;
}
