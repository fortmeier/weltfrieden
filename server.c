#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <lo/lo.h>
#include <sys/types.h>
#include <math.h>
#include <sys/time.h>
#include <assert.h>


#include "config.h"
#include "server.h"
#include "layer.h"
#include "shader.h"
#include "text.h"

extern double now;

void error(int num, const char *msg, const char *path) {
  printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

int generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data) {
  int i;

  printf("path: <%s>\n", path);
  for (i=0; i<argc; i++) {
    printf("arg %d '%c' ", i, types[i]);
    lo_arg_pp(types[i], argv[i]);
    printf("\n");
  }
  printf("\n");

  return 1;
}

void parse_showargs(lo_arg **argv, int argc, t_showargs *args) {
  int blendmode = NSA;  // default is GL_ONE_MINUS_SRC_ALPHA

  double when = (double) argv[0]->i + ((double) argv[1]->i / 1000000.0);
#ifdef SUBLATENCY
  when -= SUBLATENCY;
#endif
  int poffset = 3;

  float cps = argv[2]->f;
  float dur = argv[3]->f;
  poffset = 4;
  debug("timing info: when, cps = %f\t%f\n", when, cps);

  char *words = (char *) argv[0+poffset];

  float r = argv[1+poffset]->f;
  float g = argv[2+poffset]->f;
  float b = argv[3+poffset]->f;
  float a = argv[4+poffset]->f;
  float x = argv[5+poffset]->f;
  float y = argv[6+poffset]->f;
  float z = argv[7+poffset]->f;
  float w = argv[8+poffset]->f;
  float rot_x = argv[9+poffset]->f;
  float rot_y = argv[10+poffset]->f;
  float rot_z = argv[11+poffset]->f;
  float width = argv[12+poffset]->f;
  float height = argv[13+poffset]->f;
  float speed = argv[14+poffset]->f;
  char *blendmode_s = (char *) argv[15+poffset];
  int level = argv[16+poffset]->i;
  char *text = (char *) argv[17+poffset];
  float fontsize = argv[18+poffset]->f;
  int charcode = argv[19+poffset]->i;

  debug("[charcode] %d", charcode);
  if (argc > 20+poffset) {
    printf("show server unexpectedly received extra parameters, maybe update weltfrieden?\n");
  }

  switch(blendmode_s[0]) {
  case 'c':  blendmode = SC; break; // src color
  case 'a':  blendmode = SA; break; // src alpha
  case 'C': blendmode = DC; break; // dst color
  case 'A': blendmode = DA; break; // dst alpha
  case 'l': blendmode = CA; break; // const alpha -- mnemonic lightness?
  case 't': blendmode = CC; break; // const color -- mnemonic tint
  case 's': blendmode = SS; break; // src saturate
  case 'x': blendmode = NSA; break; // 1 - src alpha
  case 'y': blendmode =  NSC; break; // 1 - src color
  case 'X': blendmode = NDA; break; // 1 - dst alpha
  case 'Y': blendmode = NDC; break; // 1 - dst color
  };

  args->when = when;
  args->cps = cps;
  args->dur = dur;
  args->words = words;
  args->r = r;
  args->g = g;
  args->b = b;
  args->a = a;
  args->x = x;
  args->y = y;
  args->z = z;
  args->w = w;
  args->width = width;
  args->height = height;
  args->speed = speed;
  args->blendmode = blendmode;
  args->level = level;
  args->text = text;
  args->fontsize = fontsize;
  args->charcode = charcode;
  args->rot_x = rot_x;
  args->rot_y = rot_y;
  args->rot_z = rot_z;
  return;
}

int shader_handler(const char *path, const char *types, lo_arg **argv,
                 int argc, void *data, void *user_data) {
  t_showargs args;

  parse_showargs(argv, argc, &args);

  if (strlen(args.words) > 0) {
    shaderlayer_add(args);
  }
  else {
    log_info("[server:shader] no name given\n");
  }
  return 0;
}

int text_handler(const char *path, const char *types, lo_arg **argv,
                 int argc, void *data, void *user_data) {
  t_showargs args;

  parse_showargs(argv, argc, &args);

  if (strlen(args.words) > 0) {
    textlayer_add(args);
  }
  else {
    log_info("[server:text] no text given\n");
  }

  return 0;
}


extern int server_init(void) {

  lo_server_thread st = lo_server_thread_new(OSC_PORT, error);

  lo_server_thread_add_method(st, "/shader", "iiffsffffffffffffffsisfi",
                              shader_handler,
                              NULL
			      );

  /* lo_server_thread_add_method(st, "/text", "iiffsffffffffffsisi", */
  /*                             text_handler, */
  /*                             NULL */
  /*       		      ); */


  lo_server_thread_add_method(st, NULL, NULL, generic_handler, NULL);
  lo_server_thread_start(st);

  return(1);
}
