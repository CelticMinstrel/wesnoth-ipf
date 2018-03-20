#version 120

// The alpha compositing formula
vec4 blend_alpha(vec4 base, vec4 tint) {
	vec4 result;
	result.a = tint.a + base.a * (1.0 - tint.a);
	result.rgb = (tint.rgb * tint.a + base.rgb * base.a * (1.0 - tint.a)) / result.a;
	if(result.a == 0.0) result.rgb = vec3(0.0);
	return result;
}

vec4 blend_add(vec4 base, vec4 shift) {
	return clamp(base + shift, 0.0, 1.0);
}

vec4 blend(vec4 base, vec4 tint) {
	// This should be the same as mix(base.rgb, tint.rgb, tint.a)?
	vec3 result = base.rgb * (1.0 - tint.a) + tint.rgb * tint.a;
	return vec4(result, base.a);
}

vec4 greyscale(vec4 color) {
	// gray=0.299red+0.587green+0.114blue
	color.rgb *= vec3(0.299, 0.587, 0.114);
	float grey = color.r + color.g + color.b;
	return vec4(grey,grey,grey,color.a);
}

vec4 monochrome(vec4 color, float threshold) {
	color = greyscale(color);
	float c = color.r < threshold ? 0.0 : 1.0;
	color.rgb = vec3(c,c,c);
	return color;
}

vec4 invert(vec4 color, vec3 threshold) {
	color.r = color.r > threshold.r ? 1.0 - color.r : color.r;
	color.g = color.g > threshold.g ? 1.0 - color.g : color.g;
	color.b = color.b > threshold.b ? 1.0 - color.b : color.b;
	//color.rgb = 1.0 - color.rgb;
	return color;
}

vec4 sepia(vec4 color) {
	vec4 result = color;
	result.r = min(1.0, color.r * 0.393 + color.g * 0.769 + color.b * 0.189);
	result.g = min(1.0, color.r * 0.349 + color.g * 0.686 + color.b * 0.168);
	result.b = min(1.0, color.r * 0.272 + color.g * 0.534 + color.b * 0.131);
	return result;
}

vec4 light(vec4 color, vec4 light_color) {
	//vec3 light = texture2D(lightmap, gl_TexCoord[0].st).rgb;
	light_color = (light_color - 0.5) * 2.0;
	color.rgb -= light_color.rgb;
	return color;
}

bool approx_equal(float a, float b) {
	return abs(a - b) < 0.0001;
}

int find(vec3 haystack[256], vec3 needle, int length) {
	for(int i = 0; i < min(length, haystack.length()); i++) {
		vec3 me = haystack[i];
		if(approx_equal(me.r,needle.r) && approx_equal(me.g,needle.g) && approx_equal(me.b,needle.b))
			return i;
	}
	return -1;
}

vec4 recolor(vec3 source[256], vec3 dest[256], vec4 c, int pal_size) {
	int i = find(source, c.rgb, pal_size);
	if(i >= 0) c.rgb = dest[i];
	return c;
}

struct team_color {
	vec3 min, max, mid;
};

vec4 recolor(vec3 source[256], team_color dest, vec4 c, int pal_size) {
	int i = find(source, c.rgb, pal_size);
	if(i >= 0) {
		vec3 ref = pal_size > 0 ? source[0] : vec3(0,0,0);
		float ref_avg = (ref.r + ref.g + ref.b) / 3;
		float old_avg = (c.r + c.g + c.b) / 3;
		if(ref_avg > 0.0 && old_avg <= ref_avg) {
			float old_ratio = old_avg / ref_avg;
			c.r = old_ratio * dest.mid.r + (1 - old_ratio) * dest.min.r;
			c.g = old_ratio * dest.mid.g + (1 - old_ratio) * dest.min.g;
			c.b = old_ratio * dest.mid.b + (1 - old_ratio) * dest.min.b;
		} else if(ref_avg < 1.0) {
			float old_ratio = (1.0 - old_avg) / (1.0 - ref_avg);
			c.r = old_ratio * dest.mid.r + (1 - old_ratio) * dest.max.r;
			c.g = old_ratio * dest.mid.g + (1 - old_ratio) * dest.max.g;
			c.b = old_ratio * dest.mid.b + (1 - old_ratio) * dest.max.b;
		} else {
			// Should never get here.
			// Would imply old_avg > 1.0.
			// Should we do something here just in case?
		}
		c.rgb = clamp(c.rgb, 0.0, 1.0);
	}
	return c;
}





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




// All shaders have at least one sampler!
uniform sampler2D base_tex;
