#ifndef _WF_SHADER_H
#define _WF_SHADER_H

#include "config.h"
#include "dbg.h"

#include "gl_env.h"
#include "layer.h"
#include "t_shader.h"

#define SHADERLAYER_TYPE_FLAG 1

void shaderlayer_add(t_showargs args);
void shaderlayer_apply(layer *l);
void shaderlayer_finish(layer *l);
void shaderlayer_init(layer *l);
void shaderlayer_read_cache(layer *cached, layer *uncached);

layer *shaderlayer_new();
#endif
