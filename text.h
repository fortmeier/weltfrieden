#ifndef _WF_TEXT_H
#define _WF_TEXT_H

#include "config.h"
#include "dbg.h"
#include "gl_env.h"

#include "layer.h"
#include "t_text.h"

#define TEXTLAYER_TYPE_FLAG 2

void textlayer_add(t_showargs args);
void textlayer_apply(layer* l);
void textlayer_finish(layer* l);
void textlayer_init(layer* l);
void textlayer_read_cache(layer *cached, layer *uncached);
void textlayer_load_shaders();

layer *textlayer_new();
#endif
