#version 330

uniform float now;
uniform float shape;
uniform float elapsed;
uniform float cutoff;
uniform float hcutoff;

layout(location = 0) out vec4 frag_color;
in vec4 gl_FragCoord;

int julia(vec2 z, int iters) {
  int i;
  vec2 mu;

  mu = z;

  for(i=0; i<iters; i++) {
    if(z.x >= 2.0f) break;

    z = vec2(z.x*z.x - z.y*z.y, 2.0f*z.x*z.y);

    z += mu;
  }

  if(i == iters) return -1;

  return i;
}

void main(void) {
  float r, g, b;
  int bailout;
  int iters = int(shape);

  vec2 uv = gl_FragCoord.xy;
  uv.x *= cutoff;
  uv.y *= hcutoff;
  bailout = julia(uv, iters);


  if(bailout == -1) {
    r = g = b = 0.75f;
  } else {
    r = 0.0f;
    g = 1.0f*bailout/shape;
    b = 0.0f;
  }

  frag_color = vec4(r, g, b, 1.0f);
}
