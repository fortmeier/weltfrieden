// #version 330

uniform float x;
uniform float y;

uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform vec2 offset;

uniform float width;
uniform float height;
uniform float speed;
uniform vec4 color;
uniform vec4 position;
uniform vec3 origin;
uniform vec3 rotation;


uniform float alpha;

#define M_PI 3.1415926535897932384626433832795



void main() {
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  float n = 1 - min(elapsed / (dur/cps) * speed, 1);

  gl_FragColor = vec4(1.0,x,y,alpha);
}
