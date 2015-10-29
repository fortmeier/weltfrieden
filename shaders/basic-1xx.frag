#version 120

uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform vec4 color;
uniform vec4 position;
uniform float scale;
uniform float speed;

//uniform sampler2D tex;

void main()
{
  float n = elapsed / (dur/cps) * speed;
  gl_FragColor =  vec4(color.rgb, n);
}
