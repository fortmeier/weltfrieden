#ifndef _WF_TEXT_H
#define _WF_TEXT_H

#include "config.h"
#include "dbg.h"
#include "gl_env.h"

#include "layer.h"

void textlayer_add(t_showargs args);
void textlayer_apply(layer* l, int even);
void textlayer_finish(layer* l);
void textlayer_init(layer* l);

layer *textlayer_new();
#endif
