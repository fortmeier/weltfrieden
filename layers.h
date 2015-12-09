#ifndef _WF_LAYERS_H
#define _WF_LAYERS_H

#include "config.h"
#include "dbg.h"
#include "gl_env.h"

#include "queue.h"

void layers_init();
void layers_destroy();
void layers_apply();
void layers_cleanup();
void layers_finish(int even);
void layers_redraw_scribble();
void layers_clear_cache();

#endif
