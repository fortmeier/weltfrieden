#ifndef _WF_LAYERS_H
#define _WF_LAYERS_H

#include "config.h"
#include "dbg.h"
#include "gl_env.h"

#include "queue.h"

typedef struct image {
  char* name;
  GLuint texture;
} image_t;

void layers_init(int num_workers);
void layers_add(t_showargs args);
void layers_destroy();
void layers_apply();
void layers_cleanup();
void layers_finish(int even);
void layers_clear_cache();

#endif
