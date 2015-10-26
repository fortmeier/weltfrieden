#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include <scribble.h>

extern double now;
extern int shader_lvl;
extern float res[2];

layer *scribblelayer_new() {
  layer *l = layer_new();

  if (l == NULL) {
    return(NULL);
  }

  l->scribble = malloc(sizeof(scribblelayer));
  l->is_text = 0;
  l->is_scribble = 1;

  return (l);
}

void scribblelayer_init(layer *l) {
}

void scribblelayer_add(t_showargs args) {
}

void scribblelayer_apply(layer *l, int even) {
}

void scribblelayer_finish(layer *l) {
}
