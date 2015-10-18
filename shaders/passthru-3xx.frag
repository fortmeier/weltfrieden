#version 330

uniform vec2 res;
uniform sampler2D tex;

in vec4 gl_FragCoord;
in vec2 texcoord;

layout(location = 0) out vec4 frag_color;

void main() {
  vec4 color = texture(tex, vec2(texcoord.x, 1 - texcoord.y));
  frag_color = color;
}
