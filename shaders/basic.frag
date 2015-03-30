#version 330


uniform float iGlobalTime;

out vec4 fragColor;


void main()
{
    fragColor = vec4(1.0, 0.0, iGlobalTime, 1.0);
}
