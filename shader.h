#ifndef _WF_SHADER_H
#define _WF_SHADER_H

#include "config.h"
#include "dbg.h"

enum shaderstate {UNUSED, UNINITIALIZED, INITIALIZED};

enum blendmodes {
  NSA, // 1 - src alpha
  NSC, // 1 - src color
  SC, // src color
  SA, // src alpha
  SS, // src saturate
  NDA, // 1 - dst alpha
  NDC, // 1 - dst color
  DC, // dst color
  DA, // dst alpha
  CA, // const alpha
  CC // const color
};

typedef struct {
  double when;
  float cps;
  char *samplename;
  float offset;
  float start;
  float end;
  float speed;
  float pan;
  float velocity;
  int blendmode;
  float cutoff;
  float resonance;
  float accelerate;
  float shape;
  int kriole_chunk;
  float gain;
  int cutgroup;
  float delay;
  float delaytime;
  float delayfeedback;
  float crush;
  int coarse;
  float hcutoff;
  float hresonance;
  float bandf;
  float bandq;
  char unit;
  int sampleloop;
} t_playargs;

typedef struct shader_t
{
  enum shaderstate state;
  t_playargs args;
  char *filename;
  char *filecontent;
  float duration;
  double when;

  unsigned int progid;
  unsigned int shaderid;
  struct shader_t *next, *prev;
} shader;

#include "queue.h"


void shaders_init();
void shaders_destroy();
void shaders_apply();
void shaders_cleanup();

void shader_add(shader *s);
void shader_apply(shader *s);
shader *shader_new();

#endif
