#version 330

uniform float now;
uniform vec2 res;
uniform float gain;
uniform float shape;
uniform float speed;
uniform float offset;

in vec4 gl_FragCoord;
layout(location = 0) out vec4 frag_color;

mat4 rotation_matrix(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
              oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
              oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
              0.0,                                0.0,                                0.0,                                1.0);
}

void main() {
  mat4 rot = rotation_matrix(vec3(0.5,0.5,0.5), shape);
  vec4 color = vec4(0.2, 0.8,0.5 + 0.5 * sin(now)*shape, gain);
  ivec4 m = (ivec4(gl_FragCoord) + ivec4(offset,offset,offset, 0)) % (4*ivec4(shape, shape, shape, shape));

  if (m.x > shape && m.y > shape) {
    float c = gl_FragCoord.x;
    frag_color = vec4(0.95,0.95,0.95,1);
  }
  else {
    frag_color = 1 - color;
  }
}
