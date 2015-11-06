#version 330

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
//  float pi = 3.141592;

  // mat4 rot = rotation_matrix(vec3(0,0,1.0), 0.09*pi); // distance field?
  vec4 pos = vec4(position.xy, 0, 0);

//  float angle = 0.0*pi;
//  float s = scale / 2;
 vec2 top_left = vec2(-width/2,-height/2);
//  vec2 top_right = rotate( vec2(s, -s), angle);
//  vec2 bottom_left = rotate( vec2(-s, s), angle);
 vec2 bottom_right = vec2(width/2, height/2);


  top_left += pos.xy;
  bottom_right += pos.xy;

  float n = 1 - min(elapsed / (dur/cps) * speed, 1);

//  bvec4 xy_plane = bvec4(greaterThan(uv, top_left), lessThan(uv, bottom_right));

  //edges
  /* vec2 up = top_right - top_left; */
  /* vec2 left = bottom_left - top_left; */
  /* vec2 right = bottom_right - top_right; */
  /* vec2 bottom = bottom_right - bottom_left; */

  /* bvec4 hits = dot(uv, up); */


  vec2 box = step(top_left, uv) * step(uv, bottom_right);

  float is_plane = box.x * box.y;
  frag_color = mix(vec4(0,0,0,0), color, is_plane);
}
