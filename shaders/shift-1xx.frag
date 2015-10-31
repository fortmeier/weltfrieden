// #version 120

uniform float elapsed;
uniform float cps;
uniform float dur;
uniform vec2 res;

uniform vec4 color;
uniform vec4 position;
uniform float scale;
uniform float speed;

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

void main()
{
  vec2 uv = gl_FragCoord.xy / res.xy;
  float pi = 3.141592;
  mat4 rot = rotation_matrix(vec3(0.5,0.5,0.0), position.z*pi);

  uv = vec2((rot*vec4(uv.x, uv.y, 0.0, 0.0)).xy);

  float n = elapsed / (dur / cps);
  float c = uv.x + (0.5 + 0.5 * ( sin(n)));
  vec3 chroma = color.rgb;

  if ((uv.x >= position.x) && (uv.x <= position.y)) {
    vec4 color = vec4(c, c, c, 1.0);
    gl_FragColor =  vec4(vec3(mix(vec3(c), chroma, scale)), 1.0);
  }
  else {
    gl_FragColor = vec4(0,0,0,0);
  }
}
