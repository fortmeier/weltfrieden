#ifndef _WF_SCRIBBLE_H
#define _WF_SCRIBBLE_H

#include "config.h"
#include "dbg.h"

#include "gl_env.h"
#include "layer.h"

void scribblelayer_add(t_showargs args);
void scribblelayer_apply(layer *l, int even);
void scribblelayer_finish(layer *l);
void scribblelayer_init(layer *l);

layer *scribblelayer_new();
#endif
