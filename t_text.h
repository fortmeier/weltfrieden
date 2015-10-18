#ifndef _WF_TEXTLAYER_H
#define _WF_TEXTLAYER_H

typedef struct textlayer_t {
  char* text;
  GLuint res[2];
  GLuint bearing[2];
  GLuint advance;
} textlayer;

#endif
