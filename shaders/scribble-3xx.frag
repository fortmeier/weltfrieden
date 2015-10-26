#version 330

uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform vec2 cursor;
uniform float gain;
uniform float scale;
uniform float speed;
uniform vec4 color;
uniform vec4 position;

uniform sampler2D fbotex;

in vec4 gl_FragCoord;
in vec2 texcoord;

layout(location = 0) out vec4 frag_color;

void main() {
  float n = elapsed / (dur/cps);
  float m = length(res);
  float d = 1 - distance(gl_FragCoord.xy, cursor) / m;

  vec4 fbo = texture(fbotex, vec2(texcoord.x, 1 - texcoord.y));
  frag_color = (color * pow(d,1/scale)) + (fbo);
}
