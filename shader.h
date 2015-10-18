#ifndef _WF_SHADER_H
#define _WF_SHADER_H

#include "config.h"
#include "dbg.h"

#include "gl_env.h"
#include "layer.h"

void shaderlayer_add(t_showargs args);
void shaderlayer_apply(layer *l, int even);
void shaderlayer_finish(layer *l);
void shaderlayer_init(layer *l);

layer *shaderlayer_new();
#endif
