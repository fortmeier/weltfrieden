#version 120

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


mat4 rotation_matrix(vec3 axis, float angle)
{
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
  mat4 rot = rotation_matrix(vec3(0.5,0.5,0.5), scale);
  vec4 c = vec4(0.2, 0.8,0.5 + 0.5 * sin(elapsed)*scale, color.a);
  int min_scale = 4;
  vec4 ifrag = vec4(gl_FragCoord);
  vec4 icol = vec4(vec3(color.rgb), 0);
  vec4 iscale = min_scale * vec4(scale);
    //  ivec4 m = mod((ifrag + ivec4(color.x,color.y,color.z, 0)), (4*ivec4(scale, scale, scale, scale)));

  vec4 m = mod(ifrag + icol, iscale);
  if (m.x > scale && m.y > scale) {
    float c = gl_FragCoord.x;
    gl_FragColor = vec4(0.95,0.95,0.95,1);
  }
  else {
    gl_FragColor = 1 - color;
  }

  // gl_FragColor = vec4(vec3(icol).rgb,0.5);
}
