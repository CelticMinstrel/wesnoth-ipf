#version 120

mat3 translate(float x, float y) {
	mat3 trans = mat3(1);
	trans[2][0] = x;
	trans[2][1] = y;
	return trans;
}

mat3 flip(bool h, bool v) {
	mat3 trans = mat3(1);
	if(h) trans[0][0] *= -1.0;
	if(v) trans[1][1] *= -1.0;
	return trans;
}
/*
[Jul 08@2:44:57pm] DeFender1031: 90 is x2=h-1-y1, y2=x1
[Jul 08@2:44:59pm] DeFender1031: 180 is x2=w-1-x1, y2=h-1-y1
[Jul 08@2:45:00pm] DeFender1031: -90 is x2=y1 y2=w-1-x1

90 degrees is dest.xy = vec2(1 - dest.y, dest.x)
-90degrees is dest.xy = vec2(dest.y, 1 - dest.x)

180 degrees is dest.xy = 1 - dest.yx

*/
mat3 rotate(vec2 pivot, float angle) {
	angle = mod(angle, 360.0);
	if(angle == 180.0) return flip(true, true);
	mat3 trans = mat3(1);
	float theta = radians(angle);
	float c = cos(theta), s = sin(theta);
	trans[0][0] = c;  trans[1][0] = s;
	trans[0][1] = -s; trans[1][1] = c;
	mat3 offset = translate(pivot.x, pivot.y);
	return -offset * trans * offset;
}

mat3 shear(float x, float y) {
	mat3 trans = mat3(1);
	trans[1][0] = x;
	trans[0][1] = y;
	return trans;
}

mat3 scale(float x, float y) {
	mat3 trans;
	trans[0][0] = x;
	trans[1][1] = y;
	return trans;
}
