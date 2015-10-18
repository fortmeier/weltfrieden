#include "queue.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int queue_size(layer *queue) {
  int result = 0;
  while (queue != NULL) {
    result++;
    queue = queue->next;
    if (result > 4096) {
      printf("whoops, big queue\n");
      break;
    }
  }
  return(result);
}

void queue_add(layer **queue, layer *new) {
  int added = 0;
  if (*queue == NULL) {
    *queue = new;
    added++;
  }
  else {
    layer *tmp = *queue;
    assert(tmp->prev == NULL);

    int i =0;
    while (1) {
      if (tmp->when > new->when) {
        // insert in front of later event
        new->next = tmp;
        new->prev = tmp->prev;
        if (new->prev != NULL) {
          new->prev->next = new;
        }
        else {
          *queue = new;
        }
        tmp->prev = new;

        added++;
        break;
      }

      if (tmp->next == NULL) {
        // add to end of queue
        tmp->next = new;
        new->prev = tmp;
        added++;
        break;
      }
      ++i;
      tmp = tmp->next;
    }
  }

  assert(added == 1);
}


void queue_remove(layer **queue, layer *old) {
  if (old->prev == NULL) {
    *queue = old->next;
    if (*queue  != NULL) {
      (*queue)->prev = NULL;
    }
  }
  else {
    old->prev->next = old->next;

    if (old->next) {
      old->next->prev = old->prev;
    }
  }
  old->state = UNUSED;
}

layer *queue_next(layer **queue, double now) {
  layer *result = NULL;

  if (*queue != NULL && (*queue)->when <= now) {
    result = *queue;
    *queue = (*queue)->next;
    if ((*queue) != NULL) {
      (*queue)->prev = NULL;
    }
  }

  return(result);
}
