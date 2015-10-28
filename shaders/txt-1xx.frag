#version 120

uniform sampler2D tex;

uniform float elapsed;
uniform float dur;
uniform float cps;
uniform vec2 res;

uniform vec3 text_color;

varying vec2 o_tex_coord;

void main()
{
  float a = texture2D(tex, o_tex_coord.xy).r;

  float n = elapsed / (dur/cps);

  vec3 c = (text_color * a);

  if (a > 0) {
    gl_FragColor = vec4(c, 1.0);
  }
  else {
    discard;
  }
}
