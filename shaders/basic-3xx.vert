#version 330

uniform vec2 res;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


layout(location = 0) in vec3 vp;
layout(location = 1) in vec2 v_texcoord;

out vec2 texcoord;

void main() {
  texcoord = v_texcoord;
  gl_Position = vec4(vp, 1.0) ; //projection*(view*(model*vec4(vp,1.0)));
}
