#version 330

uniform float width;
uniform float height;
uniform vec3 rotation;
uniform vec4 position;

layout(location = 0) in vec3 vp;

out vec2 tex_coord;

vec2 rotate(vec2 p, float angle) {
  vec2 rot;

  rot.x = p.x * cos(angle) - p.y * sin(angle);
  rot.y = p.x * sin(angle) + p.y * cos(angle);
  return rot;
}

void main() {
  gl_Position = vec4((2*position.xy) - vec2(1,1),0,0) + vec4(rotate(vp.xy, rotation.x), 0, 1.0) * vec4(width, height, 0, 1.0);
  tex_coord = (vec2(0.5,0.5) + ( (gl_Position.xy/2) / vec2(width, height) ) );
}
