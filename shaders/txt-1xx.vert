#version 120

varying vec2 o_tex_coord;

void main() {
  o_tex_coord = gl_Vertex.zw;
  gl_Position       = vec4(gl_Vertex.xy, 0.0, 1.0);
}
