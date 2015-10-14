#version 120

uniform float now;
uniform float elapsed;
uniform vec2 res;
uniform float gain;
uniform float shape;
uniform float speed;

uniform sampler2D tex;

void main()
{
vec2 uv = gl_FragCoord.xy / res.xy;

gl_FragColor =  vec4(1.0, 1.0 , 1.0, max((1. - elapsed)*gain, 0.));
}
