#version 330

uniform sampler2D tex;
uniform sampler2D fbotex;

uniform float elapsed;
uniform float dur;
uniform float cps;
uniform vec2 res;

uniform vec3 text_color;

in vec4 gl_FragCoord;
in vec2 o_tex_coord;

layout(location = 0) out vec4 frag_color;

#define M_PI 3.1415926535897932384626433832795

vec2 rotate(vec2 p, float angle) {
  vec2 rot;

  rot.x = p.x * cos(angle) - p.y * sin(angle);
  rot.y = p.x * sin(angle) + p.y * cos(angle);
  return rot;
}

void main()
{

  vec2 tex_coord = rotate(o_tex_coord, 0.4*M_PI/2);
  float a = texture(tex, tex_coord).r;


  vec4 fbo = texture(fbotex, (gl_FragCoord.xy / res));
  float n = elapsed / (dur/cps);

  vec3 c = a * text_color;

  if (a > 0) {
    frag_color = vec4(c, 1.0);
  }
  else {
    discard;
  }
}
