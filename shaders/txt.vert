#version 330
uniform vec2 res;

layout(location = 0 ) in vec4 vertex;

out vec2 o_tex_coord;

void main() {
  o_tex_coord = vertex.zw;
  gl_Position       = vec4(vertex.xy, 0.0, 1.0);
}
