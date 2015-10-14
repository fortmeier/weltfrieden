#ifndef _WF_QUEUE_H
#define _WF_QUEUE_H

#include "shader.h"

int queue_size(shader *queue);

void queue_add(shader **queue, shader *new);

void queue_remove(shader **queue, shader *old);

shader *queue_next(shader **queue, double now);

#endif
