
#line 2 $IDX|

vec4 $FCN|(sampler2D tex, vec2 tc) {
	vec4 color = texture2D(tex, tc);
	$COLOR_MUTATIONS|
	#line 8 $IDX|
	return color;
}
