#version 330

uniform float iGlobalTime;
uniform float iTime;
uniform vec2 iResolution;
uniform float gain;
uniform float shape;
uniform float speed;


in vec4 gl_FragCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
  fragColor =  vec4(1., 1. , 1., max(1. - iTime, 0.) * gain);
}
