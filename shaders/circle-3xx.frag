#version 330

uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform float gain;
uniform float width;
uniform float height;
uniform float speed;
uniform vec4 color;
uniform vec4 position;
uniform vec2 offset;
/* uniform vec3 rotation; */

out vec4 frag_color;

vec2 rotate(vec2 p, float angle) {
  vec2 rot;

  rot.x = p.x * cos(angle) - p.y * sin(angle);
  rot.y = p.x * sin(angle) + p.y * cos(angle);
  return rot;
}

void main() {
  vec2 uv = (gl_FragCoord.xy - offset) / res.xy;

  float radius = width / 2;
  float n = 1 - min(elapsed / (dur/cps) * speed, 1);

  float circ = distance(uv, position.xy);

  float box = step(circ, radius);

  frag_color = mix(vec4(0,0,0,0), color, box);
}
