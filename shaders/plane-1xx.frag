
uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform float gain;
uniform float scale;
uniform float speed;
uniform vec4 color;
uniform vec4 position;


void main() {
  vec2 uv = gl_FragCoord.xy / res.xy;
  vec2 top_left = position.xy - vec2(scale/2.0, scale/2.0);
  vec2 bottom_right = position.xy + vec2(scale/2.0, scale/2.0);

  float n = elapsed / (dur/cps) * speed;
  
  bvec4 xy_plane = bvec4(greaterThan(uv, top_left), lessThan(uv, bottom_right));
  bool is_plane = all(xy_plane);

  gl_FragColor = vec4(color.rgb, color.a * float(is_plane) * n);
}
