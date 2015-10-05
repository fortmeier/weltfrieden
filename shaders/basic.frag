#version 330


uniform float iGlobalTime;
uniform float iTime;
uniform vec2 iResolution;
uniform float gain;
uniform float shape;
uniform float speed;

in vec4 gl_FragCoord;
//out vec4 fragColor;
uniform sampler2D tex;
layout(location = 0) out vec4 fragColor;


void main()
{
 vec2 uv = gl_FragCoord.xy / iResolution.xy;
// float time = 0.5 + 0.3 * sin(iGlobalTime) + 0.2 * cos(iGlobalTime*3);
 fragColor =  vec4(1.0, 1.0 , 1.0, max(1 - iTime, 0)*gain);
}