#version 330

uniform sampler2D tex;

uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform vec2 offset;

uniform float width;
uniform float height;
uniform float speed;
uniform vec4 color;
uniform vec4 position;
uniform vec3 origin;
uniform vec3 rotation;

#define M_PI 3.1415926535897932384626433832795

in vec2 tex_coord;

out vec4 frag_color;

vec2 rotate(vec2 p, float angle) {
  vec2 rot;

  rot.x = p.x * cos(angle) - p.y * sin(angle);
  rot.y = p.x * sin(angle) + p.y * cos(angle);
  return rot;
}

void main() {
  float angle = rotation.x*M_PI/2;
  vec2 uv = rotate(((tex_coord.xy - offset)) + (vec2(0.5,0.5) - position.xy), angle);
  vec4 tcol = texture(tex, vec2(uv.x, 1 - uv.y));
  float n = 1 - min(elapsed / (dur/cps) * speed, 1);

  frag_color = vec4(tcol.rgb, color.a * n);
}
