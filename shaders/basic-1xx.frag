#version 120

uniform float iGlobalTime;
uniform float iTime;
uniform vec2 iResolution;
uniform float gain;
uniform float shape;
uniform float speed;

uniform sampler2D tex;

void main()
{
 vec2 uv = gl_FragCoord.xy / iResolution.xy;

 gl_FragColor =  vec4(1.0, 1.0 , 1.0, max((1. - iTime)*gain, 0.));
}
