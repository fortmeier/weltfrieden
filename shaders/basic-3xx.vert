#version 330

uniform float width;
uniform float height;
uniform vec3 origin;
uniform vec3 rotation;
uniform vec4 position;

layout(location = 0) in vec3 vp;

out vec2 tex_coord;

#define M_PI 3.1415926535897932384626433832795


vec2 rotate(vec2 p, float angle) {
  vec2 rot;

  rot.x = p.x * cos(angle) - p.y * sin(angle);
  rot.y = p.x * sin(angle) + p.y * cos(angle);
  return rot;
}

void main() {
  // map input range (0 to 1) to vertex coordinates (-1 to 1)
  vec2 pos = (2*position.xy) - vec2(1.0, 1.0);
  vec2 origin = origin.xy;
  float angle = rotation.x*M_PI*2;
  vec2 scale = vec2(width, height);

  vec2 rotation_origin = origin + (vp.xy * scale);
  vec2 rotated = rotate(rotation_origin, angle);

  gl_Position = vec4((pos + rotated), 0.0, 1.0);
  tex_coord = (vec2(0.5,0.5) + ( (gl_Position.xy/2) / vec2(width, height) ) );
}
