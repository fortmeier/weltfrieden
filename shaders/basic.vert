#version 330

//uniform float iGlobalTime;
//uniform vec2 iResolution;

//layout(location = 0)
in vec4 vert;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = vert * 10.0;
    //gl_Position = projection * view * model * vert;
}
