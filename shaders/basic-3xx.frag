#version 330

uniform float now;
uniform float elapsed;
uniform vec2 res;
uniform float gain;
uniform float shape;
uniform float speed;

in vec4 gl_FragCoord;
layout(location = 0) out vec4 frag_color;

void main()
{
  frag_color =  vec4(1., 1. , 1., max(1. - elapsed, 0.) * gain);
}
