#version 330

uniform float now;
uniform float elapsed;
uniform vec2 res;
uniform float gain;
uniform float shape;
uniform float speed;
uniform float begin;
uniform float end;
uniform float offset;

in vec4 gl_FragCoord;
layout(location = 0) out vec4 frag_color;

void main() {
  vec2 z;

  z.x = 3.0 * (gl_FragCoord.x - 0.5);
  z.y = 2.0 * (gl_FragCoord.y - 0.5);

  int i;
  int iter = int(offset);
  for(i=0; i<iter; i++) {
    float x = (z.x * z.x - z.y * z.y) + begin;
    float y = (z.y * z.x + z.x * z.y) + end;

    if((x * x + y * y) > 4.0) break;
    z.x = x;
    z.y = y;
  }

  float index = i == iter ? 0.0 : (float(i % iter) / float(iter));

  frag_color = vec4(i, i, i, max(1 - elapsed, 0));
}
