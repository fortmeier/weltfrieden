#version 330

uniform float now;
uniform float dur;
uniform float cps;
uniform float elapsed;
uniform vec2 res;
uniform vec2 offset;

uniform float width;
uniform float height;
uniform float speed;
uniform vec4 color;
uniform vec4 position;
uniform vec3 rotation;

#define M_PI 3.1415926535897932384626433832795

out vec4 frag_color;

vec2 rotate(vec2 p, float angle) {
  vec2 rot;

  rot.x = p.x * cos(angle) - p.y * sin(angle);
  rot.y = p.x * sin(angle) + p.y * cos(angle);
  return rot;
}

// algorithm from http://stackoverflow.com/a/1968345
int get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        return 1;
    }

    return 0; // No collision
}

void main() {
  vec2 uv = (gl_FragCoord.xy - offset) / res.xy;
  vec4 pos = vec4(position.xy, 0, 0);

  float angle = rotation.x*M_PI/2;

  vec2 top_left = rotate(vec2(-width/2,-height/2), angle);
  vec2 top_right = rotate(vec2(width/2, -height/2), angle);
  vec2 bottom_left = rotate(vec2(-width/2, height/2), angle);
  vec2 bottom_right = rotate(vec2(width/2, height/2), angle);

  top_left += pos.xy;
  bottom_right += pos.xy;
  top_right += pos.xy;
  bottom_left += pos.xy;

  float n = 1 - min(elapsed / (dur/cps) * speed, 1);

  int hits_up = get_line_intersection(0,0,uv.x,uv.y,top_left.x, top_left.y, top_right.x, top_right.y);
  int hits_right = get_line_intersection(0,0,uv.x,uv.y,top_right.x, top_right.y, bottom_right.x, bottom_right.y);
  int hits_bottom = get_line_intersection(0,0,uv.x,uv.y,bottom_right.x, bottom_right.y, bottom_left.x, bottom_left.y);
  int hits_left = get_line_intersection(0,0,uv.x,uv.y,bottom_left.x, bottom_left.y, top_left.x, top_left.y);

  int hits = hits_up + hits_right + hits_bottom + hits_left;

  if (hits == 1) {
    frag_color = color;
  }
  else {
    frag_color = vec4(0,0,0,0);
  }
}
