#version 330

uniform float iGlobalTime;
//uniform vec2 param;
uniform float shape;
//uniform vec2 viewPos;
// uniform float viewZoom;
// uniform float viewRatio;
uniform float iTime;
uniform float cutoff;
uniform float hcutoff;
layout(location = 0) out vec4 FragmentColor;


in vec4 gl_FragCoord;

int julia(vec2 z, int iters) {
	int i;
	vec2 mu;

        mu = z;

	for(i=0; i<iters; i++) {
		if(z.x >= 2.0f) break;

		z = vec2(z.x*z.x - z.y*z.y, 2.0f*z.x*z.y);

		z += mu;
	}

	if(i == iters) return -1;

	return i;
}

void main(void) {
	float r, g, b;
	int bailout;
        int iters = int(shape);

        vec2 uv = gl_FragCoord.xy;
        uv.x *= cutoff;
        uv.y *= hcutoff;
	bailout = julia(uv, iters);


	if(bailout == -1) {
		r = g = b = 0.75f;
	} else {
          if(bailout%32 == int(iTime*32)) {
          	r = 1.0f;
                        	} else {
			r = 0.0f;
                        		}
		g = 1.0f*bailout/shape;
		b = 0.0f;
	}


        	FragmentColor = vec4(r, g, b, 1.0f);
                FragmentColor = vec4(cross(vec3(gl_FragCoord.xy,0.7),FragmentColor.xyz), 1.0);
       	// FragmentColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
}
