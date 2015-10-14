#version 330

uniform float now;
uniform vec2 res;
uniform float gain;

in vec4 gl_FragCoord;
out vec4 frag_color;

void main()
{
  frag_color = vec4(1., 1., 1., gain);
}
