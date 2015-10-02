#version 330


uniform float iGlobalTime;
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
 float time = 0.5 + 0.5 * sin(iGlobalTime);
 fragColor =  vec4(gain*time*speed, gain*time*speed ,gain*time*speed,0.1);
}