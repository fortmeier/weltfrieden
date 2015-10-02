#define MAXSHADERLAYERS 20


enum shaderstate {UNUSED, UNINITIALIZED, INITIALIZED};
typedef struct
{
  enum shaderstate state;
  float gain; // maps to opacity of shader layer
  float shape; // maps time to current color value
  float speed;
  char *filename;
  char *filecontent;
  float duration;
  float end;
  double when;

  unsigned int progId;
  unsigned int shaderId;
} shader;


void applyShaderLayer(unsigned int i);
void addShaderLayer(shader s);
void useShaderLayer(shader *s);

void removeDeadLayers();
