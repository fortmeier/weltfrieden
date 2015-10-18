#version 330

uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform float gain;
uniform float scale;
uniform float speed;
uniform vec4 color;
uniform vec4 position;

uniform sampler2D fbotex;

in vec4 gl_FragCoord;
in vec2 texcoord;

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
  vec4 fbo = texture(fbotex, vec2(texcoord.x, 1 - texcoord.y));
  mat4 rot = rotation_matrix(vec3(0.5,0.5,0.5), scale);
  ivec4 m = (ivec4(gl_FragCoord) + ivec4(position)) % (4*ivec4(scale, scale, scale, scale));

  float n = elapsed/(dur/cps);

  if (m.x > scale && m.y > scale) {
    frag_color = mix(color, fbo, n);
  }
  else {
    frag_color = mix(1 - color, fbo, 1);
  }
}
