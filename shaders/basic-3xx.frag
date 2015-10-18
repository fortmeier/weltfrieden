#version 330

uniform float now;
uniform float elapsed;
uniform float cps;
uniform float dur;
uniform vec2 res;
uniform vec4 color;
uniform vec4 pos;
uniform float speed;
uniform float scale;


uniform sampler2D fbotex;

in vec4 gl_FragCoord;
layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 fbo_color;
void main()
{
  float pi = 3.141592;
  vec4 fbo = texture(fbotex, gl_FragCoord.xy / res);
  vec4 diff_color = vec4(color.rgb, cos(pi*elapsed/(dur/cps))*color.a);
  frag_color = mix(fbo, diff_color, scale);
  /* fbo_color = cross(fbo, diff_color); */
}
