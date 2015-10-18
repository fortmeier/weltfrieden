#ifndef _WF_QUEUE_H
#define _WF_QUEUE_H

#include "layer.h"

int queue_size(layer *queue);

void queue_add(layer **queue, layer *new);

void queue_remove(layer **queue, layer *old);

struct layer_t *queue_next(layer **queue, double now);

#endif
