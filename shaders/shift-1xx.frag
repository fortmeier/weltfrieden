#version 120

uniform float iGlobalTime;
uniform vec2 iResolution;
uniform float gain;
uniform float shape;
uniform float speed;
uniform float start;
uniform float end;
uniform float pan;

//in vec4 gl_FragCoord;
uniform sampler2D tex;
//layout(location = 0) out vec4 fragColor;


mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
  vec2 uv = gl_FragCoord.xy / iResolution.xy;
  float pi = 3.141592;
  mat4 rot = rotationMatrix(vec3(0.5,0.5,0.0), pan*pi);
  uv = vec2((rot*vec4(uv.x, uv.y, 0.0, 0.0)).xy);
  float c = uv.x + (0.5 + 0.5 * ( sin(iGlobalTime)));
  vec3 chroma = vec3(uv.x, uv.y, .5 + .5 * sin(iGlobalTime)); //, c, 1.0);
  if ((uv.x >= start) && (uv.x <= end)) {
    vec4 color = vec4(c, c, c, 1.0);
    gl_FragColor =  vec4(vec3(mix(vec3(c), chroma, shape)), 1.0);
  }
  else {
    gl_FragColor = vec4(0,0,0,0);
  }
}
