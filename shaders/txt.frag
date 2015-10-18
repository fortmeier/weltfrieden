#version 330

uniform sampler2D tex;
uniform sampler2D fbotex;

uniform float elapsed;
uniform float dur;
uniform float cps;
uniform vec2 res;

uniform vec3 text_color;

in vec4 gl_FragCoord;
in vec2 o_tex_coord;

layout(location = 0) out vec4 frag_color;

void main()
{
  float a = texture(tex, o_tex_coord).r;

  vec4 fbo = texture(fbotex, (gl_FragCoord.xy / res));
  float n = elapsed / (dur/cps);

  vec3 c = a * text_color;

  frag_color = mix(vec4(((1 -
                          a) * (fbo.rgb )) + c, 1), fbo, n);
}
