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

/* in vec2 texcoord; */
in vec4 gl_FragCoord;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 fbo_color;


float random_noise(vec2 p) {
  return fract(6791.*sin(47.*p.x+p.y*9973.));
}

float smooth_noise(vec2 p) {
  vec2 nn = vec2(p.x, p.y+1.);
  vec2 ee = vec2(p.x+1., p.y);
  vec2 ss = vec2(p.x, p.y-1.);
  vec2 ww = vec2(p.x-1., p.y);
  vec2 cc = vec2(p.x, p.y);

  float sum = 0.;
  sum += random_noise(nn)/8.;
  sum += random_noise(ee)/8.;
  sum += random_noise(ss)/8.;
  sum += random_noise(ww)/8.;
  sum += random_noise(cc)/2.;

  return sum;
}

float interpolated_noise(vec2 p) {
  float q11 = smooth_noise(vec2(floor(p.x), floor(p.y)));
  float q12 = smooth_noise(vec2(floor(p.x), ceil(p.y)));
  float q21 = smooth_noise(vec2(ceil(p.x), floor(p.y)));
  float q22 = smooth_noise(vec2(ceil(p.x), ceil(p.y)));

  vec2 s = smoothstep(0., 1., fract(p));
  float r1 = mix(q11, q21, fract(p.x));
  float r2 = mix(q12, q22, fract(p.x));

  return mix (r1, r2, s.y);
}

void main() {
  vec2 position = gl_FragCoord.xy / res.xy;
// if ((position.x>1.) || (position.y>1.)) {
//   discard;
// }

// float tiles = 4.;
// position *= tiles;
  position /= scale;
  position += elapsed / (dur / cps);
  float n = interpolated_noise(position);

  frag_color = color*vec4(vec3(n*3.141592), 1.);
}
