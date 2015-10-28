#version 120

uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform vec4 color;
uniform vec4 pos;
uniform float scale;
uniform float speed;

//uniform sampler2D tex;

void main()
{
  //vec2 uv = gl_FragCoord.xy / res.xy;

  float n = elapsed / (dur/cps) * speed;
  gl_FragColor =  vec4(color.rgb, n);
}
