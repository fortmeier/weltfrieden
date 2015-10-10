#version 330


uniform float iGlobalTime;
uniform float iTime;
uniform vec2 iResolution;
uniform float gain;
uniform float shape;
uniform float speed;
uniform float begin;
uniform float end;
uniform float offset;
uniform float cps;

in vec4 gl_FragCoord;
uniform sampler2D tex;
layout(location = 0) out vec4 fragColor;


float randomNoise(vec2 p) {
  return fract(6791.*sin(47.*p.x+p.y*9973.));
}

float smoothNoise(vec2 p) {
  vec2 nn = vec2(p.x, p.y+1.);
  vec2 ee = vec2(p.x+1., p.y);
  vec2 ss = vec2(p.x, p.y-1.);
  vec2 ww = vec2(p.x-1., p.y);
  vec2 cc = vec2(p.x, p.y);

  float sum = 0.;
  sum += randomNoise(nn)/8.;
  sum += randomNoise(ee)/8.;
  sum += randomNoise(ss)/8.;
  sum += randomNoise(ww)/8.;
  sum += randomNoise(cc)/2.;

  return sum;
}

float interpolatedNoise(vec2 p) {
  float q11 = smoothNoise(vec2(floor(p.x), floor(p.y)));
  float q12 = smoothNoise(vec2(floor(p.x), ceil(p.y)));
  float q21 = smoothNoise(vec2(ceil(p.x), floor(p.y)));
  float q22 = smoothNoise(vec2(ceil(p.x), ceil(p.y)));

  vec2 s = smoothstep(0., 1., fract(p));
  float r1 = mix(q11, q21, fract(p.x));
  float r2 = mix(q12, q22, fract(p.x));

  return mix (r1, r2, s.y);
}

void main() {
  vec2 position = gl_FragCoord.xy/iResolution.xy;
  // if ((position.x>1.) || (position.y>1.)) {
  //   discard;
  // }

  // float tiles = 4.;
  // position *= tiles;
  position /= gain;
  position += iGlobalTime * cps;
  float n = interpolatedNoise(position);

  fragColor = vec4(vec3(n*3.141592), 1.);
}
