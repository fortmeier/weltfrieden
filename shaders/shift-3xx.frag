#version 330

uniform float now;
uniform float elapsed;
uniform vec2 res;
uniform float speed;
uniform float cps;
uniform float dur;
uniform vec4 position;
uniform vec4 color;
uniform float scale;

uniform sampler2D fbotex;

in vec2 texcoord;
in vec4 gl_FragCoord;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 fbo_color;

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
  vec2 uv =  gl_FragCoord.xy / res.xy;

  float pi = 3.141592;
  mat4 rot = rotation_matrix(vec3(0.5,0.5,0.0), scale*pi);

  vec4 fbo = texture(fbotex, vec2(texcoord.x, 1 - texcoord.y));

  uv = vec2((rot*vec4(sin(uv.x*pi*position.x), (cos(uv.y*pi*position.y)), 0.0, 0.0)).xy);

  vec3 chroma = color.rgb; // vec3(color, uv.y, .5 + .5 * sin(pi*(elapsed/(dur/cps)))); //, c, 1.0);

  float n = elapsed/(dur/cps);

  if ((uv.x >= position.x) && (uv.x <= position.y)) {
    frag_color = mix(color, mix(fbo, vec4(chroma, color.a), 0.9), n);
  }
  else {
    frag_color = vec4(0,0,0, n);
  }
}
