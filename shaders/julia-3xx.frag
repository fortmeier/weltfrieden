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

in vec4 gl_FragCoord;
uniform sampler2D tex;
layout(location = 0) out vec4 fragColor;


void main() {
    vec2 z;

    z.x = 3.0 * (gl_FragCoord.x - 0.5);
    z.y = 2.0 * (gl_FragCoord.y - 0.5);

    int i;
    int iter = int(offset);
    for(i=0; i<iter; i++) {
        float x = (z.x * z.x - z.y * z.y) + begin;
        float y = (z.y * z.x + z.x * z.y) + end;

        if((x * x + y * y) > 4.0) break;
        z.x = x;
        z.y = y;
    }

    float index = i == iter ? 0.0 : (float(i % iter) / float(iter));


    fragColor = vec4(i, i, i, max(1 - iTime, 0));
}