#define MAXSHADERLAYERS 20

enum shaderstate {UNUSED, UNINITIALIZED, INITIALIZED};

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
  int vowelnum;
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
  int sample_loop;
} t_play_args;

typedef struct
{
  enum shaderstate state;
  t_play_args args;
  char *filename;
  char *filecontent;
  float duration;
  double when;

  unsigned int progId;
  unsigned int shaderId;
} shader;


void initShaders();
void uninitShaders();

void applyShaderLayer(unsigned int i);
void addShaderLayer(shader s);
void useShaderLayer(shader *s);

void removeDeadLayers();
