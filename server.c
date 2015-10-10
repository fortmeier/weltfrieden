#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <lo/lo.h>
#include <sys/types.h>
#include <math.h>
#include <sys/time.h>
#include <assert.h>

#include "server.h"
#include "config.h"

#include "shader.h"

extern float iGlobalTime;
extern double epochOffset;

extern float cube_angle;

void error(int num, const char *m, const char *path);

int trigger_handler(const char *path, const char *types, lo_arg **argv,
                    int argc, void *data, void *user_data);

int generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data);

/**/

void error(int num, const char *msg, const char *path) {
    printf("liblo server error %d in path %s: %s\n", num, path, msg);
}

/**/

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


int play_handler(const char *path, const char *types, lo_arg **argv,
                 int argc, void *data, void *user_data) {

  /* lo_timetag ts = lo_message_get_timestamp(data); */

  double when = (double) argv[0]->i + ((double) argv[1]->i / 1000000.0);
#ifdef SUBLATENCY
  when -= SUBLATENCY;
#endif
  int poffset = 2;

  float cps = argv[2]->f;
  poffset = 3;
  //printf("timing info: when, cps = %f\t%f\n", when, cps);

  char *sample_name = (char *) argv[0+poffset];

  float offset = argv[1+poffset]->f;
  float start = argv[2+poffset]->f;
  float end  = argv[3+poffset]->f;
  float speed  = argv[4+poffset]->f;
  float pan  = argv[5+poffset]->f;
  float velocity  = argv[6+poffset]->f;
  char *vowel_s = (char *) argv[7+poffset];
  float cutoff = argv[8+poffset]->f;
  float resonance = argv[9+poffset]->f;
  float accelerate = argv[10+poffset]->f;
  float shape = argv[11+poffset]->f;
  int kriole_chunk = argv[12+poffset]->i;

  float gain = argc > (13+poffset) ? argv[13+poffset]->f : 0;
  int cutgroup = argc > (14+poffset) ? argv[14+poffset]->i : 0;

  float delay = argc > (15+poffset) ? argv[15+poffset]->f : 0;
  float delaytime = argc > (16+poffset) ? argv[16+poffset]->f : 0;
  float delayfeedback = argc > (17+poffset) ? argv[17+poffset]->f : 0;

  float crush = argc > (18+poffset) ? argv[18+poffset]->f : 0;
  int coarse = argc > (19+poffset) ? argv[19+poffset]->i : 0;
  float hcutoff = argc > (20+poffset) ? argv[20+poffset]->f : 0;
  float hresonance = argc > (21+poffset) ? argv[21+poffset]->f : 0;
  float bandf = argc > (22+poffset) ? argv[22+poffset]->f : 0;
  float bandq = argc > (23+poffset) ? argv[23+poffset]->f : 0;

  char *unit_name = argc > (24+poffset) ? (char *) argv[24+poffset] : "r";
  //  int sample_loop = argc >  (25+poffset) ? argv[25+poffset]->i : 0;

  if (argc > 26+poffset) {
    printf("play server unexpectedly received extra parameters, maybe update Dirt?\n");
  }



  // default is GL_ONE_MINUS_SRC_ALPHA

  int blend_mode = NSA;



  switch(vowel_s[0]) {
  case 'c':  blend_mode = SC; break; // src color
  case 'a':  blend_mode = SA; break; // src alpha
  case 'C': blend_mode = DC; break; // dst color
  case 'A': blend_mode = DA; break; // dst alpha
  case 'l': blend_mode = CA; break; // const alpha -- mnemonic lightness?
  case 't': blend_mode = CC; break; // const color -- mnemonic tint
  case 's': blend_mode = SS; break; // src saturate
  case 'x': blend_mode = NSA; break; // 1 - src alpha
  case 'y': blend_mode =  NSC; break; // 1 - src color
  case 'X': blend_mode = NDA; break; // 1 - dst alpha
  case 'Y': blend_mode = NDC; break; // 1 - dst color
};

  int unit = -1;
  switch(unit_name[0]) {
     // rate
     case 'r': case 'R': unit = 'r'; break;
     // sec
     case 's': case 'S': unit = 's'; break;
     // cycle
     case 'c': case 'C': unit = 'c'; break;
  }



  double playTime = iGlobalTime;

  t_play_args args = {
    when,
    cps,
    sample_name,
    offset,
    start,
    end,
    speed,
    pan,
    velocity,
    blend_mode,
    cutoff,
    resonance,
    accelerate,
    shape,
    kriole_chunk,
    gain,
    cutgroup,
    delay,
    delaytime,
    delayfeedback,
    crush,
    coarse,
    hcutoff,
    hresonance,
    bandf,
    bandq,
    unit
  };


  shader s = {
    UNINITIALIZED,
    args,
    NULL,
    NULL,
    (1./cps),
    playTime,
    0, // progId
    0 // shaderId
  };
  s.filename = malloc(strlen(sample_name) + 1);
  strcpy(s.filename, sample_name);

  addShaderLayer( s );

  //free(sample_name);
  return 0;
}


/**/

#ifdef ZEROMQ
void *zmqthread(void *data){
  void *context = zmq_ctx_new ();
  void *subscriber = zmq_socket (context, ZMQ_SUB);
  void *buffer = (void *) malloc(MAXOSCSZ);

  int rc = zmq_connect (subscriber, ZEROMQ);
  lo_server s = lo_server_new("7772", error);

  lo_server_add_method(s, "/play", "iisffffffsffffififfffiffff",
		       play_handler,
		       NULL
		       );

  lo_server_add_method(s, "/play", "iisffffffsffffififfffifff",
		       play_handler,
		       NULL
		       );

  lo_server_add_method(s, "/play", "iisffffffsffffififff",
		       play_handler,
		       NULL
		       );

  lo_server_add_method(s, NULL, NULL, generic_handler, NULL);

  assert(rc == 0);
  //  Subscribe to all
  rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE,
		      NULL, 0);
  assert (rc == 0);
  while(1) {
    int size = zmq_recv(subscriber, buffer, MAXOSCSZ, 0);
    if (size > 0) {
      lo_server_dispatch_data(s, buffer, size);
    }
    else {
      printf("oops.\n");
    }
  }
  return(NULL);
}
#endif

/**/

extern int server_init(void) {

  lo_server_thread st = lo_server_thread_new(OSC_PORT, error);

  lo_server_thread_add_method(st, "/play", "iisffffffsffffififfffifffffi",
                              play_handler,
                              NULL
                             );

  lo_server_thread_add_method(st, "/play", "iisffffffsffffififfffifffff",
                              play_handler,
                              NULL
                             );

  lo_server_thread_add_method(st, "/play", "iisffffffsffffififfffiffff",
                              play_handler,
                              NULL
                             );

  lo_server_thread_add_method(st, "/play", "iisffffffsffffififff",
                              play_handler,
                              NULL
                             );

  lo_server_thread_add_method(st, "/play", "iisffffffsffffifi",
                              play_handler,
                              NULL
                             );

  // last two optional, for backward compatibility
  lo_server_thread_add_method(st, "/play", "iisffffffsffffi",
                              play_handler,
                              NULL
                             );

  lo_server_thread_add_method(st, "/play", NULL, play_handler, NULL);
  lo_server_thread_add_method(st, NULL, NULL, generic_handler, NULL);
  lo_server_thread_start(st);

  return(1);
}
