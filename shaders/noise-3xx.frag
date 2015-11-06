/*
Source:
http://www.kamend.com/2012/06/perlin-noise-and-glsl/
truncation*/
#version 330

uniform float now;
uniform float elapsed;
uniform vec2 res;
uniform float speed;
uniform float cps;
uniform float dur;
//uniform vec4 position;
//uniform vec4 color;
uniform float width;
uniform float height;


/* in vec4 gl_FragCoord; */

out vec4 frag_color;


float random_noise(vec2 p) {
  return fract(6791.*sin(47.*p.x+p.y*9973.));
}

void main() {
  vec2 position = gl_FragCoord.xy / res.xy;
  position /= 0.2;
  position += 1 - (elapsed / (dur / cps));
  float n = random_noise(position);
  vec4 color = vec4(1,0,0,1);

  frag_color = color*vec4(vec3(n*3.141592), n);
}
