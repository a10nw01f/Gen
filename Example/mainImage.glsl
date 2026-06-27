void IsPointInCircle ( vec2 point,  vec2 center,  float radius, inout vec3 color,  vec3 newColor) {
vec2 dist = (point-center);
if ((dot(dist ,dist)<(radius*radius))) {
(color=newColor);
};
}
void IsPointInRect ( vec2 point,  vec2 rectMin,  vec2 rectMax, inout vec3 color,  vec3 newColor) {
vec2 clamped = clamp(point ,rectMin ,rectMax);
if ((clamped==point)) {
(color=newColor);
};
}
void mainImage (out vec4 fragColor, in vec2 fragCoord) {
vec3 primaryColor = vec3(int(0) ,int(1) ,int(0));
vec3 bgColor = vec3(int(0) ,int(0) ,int(1));
vec3 color = bgColor;
IsPointInCircle(fragCoord ,vec2(int(200) ,int(225)) ,float(150.0000) ,color ,primaryColor);
IsPointInCircle(fragCoord ,vec2(int(200) ,int(225)) ,float(100.0000) ,color ,bgColor);
IsPointInRect(fragCoord ,vec2(int(225) ,int(0)) ,vec2(int(350) ,int(450)) ,color ,bgColor);
IsPointInRect(fragCoord ,vec2(int(250) ,int(200)) ,vec2(int(400) ,int(250)) ,color ,primaryColor);
IsPointInRect(fragCoord ,vec2(int(300) ,int(150)) ,vec2(int(350) ,int(300)) ,color ,primaryColor);
IsPointInRect(fragCoord ,vec2(int(450) ,int(200)) ,vec2(int(600) ,int(250)) ,color ,primaryColor);
IsPointInRect(fragCoord ,vec2(int(500) ,int(150)) ,vec2(int(550) ,int(300)) ,color ,primaryColor);
(fragColor=vec4(color ,int(1)));
}
