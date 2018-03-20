
#line 2 $IDX|

//#if 0
void main(void) {
	vec3 tc = vec3(gl_TexCoord[0].st, 1);
	$TEX_COORD_MUTATIONS|
	#line 8 $IDX|
	vec4 color;
	color = texture2D(base_tex, tc.st);
	$FUNCTION_CALLS|
	$COLOR_MUTATIONS|
	#line 13 $IDX|
	gl_FragColor = color;
}
//#endif

#if 0

void main(void) {
	vec4 color = texture2D(base_tex, gl_TexCoord[0].st);
	$COLOR_SAMPLERS|
	$COLOR_MUTATIONS|
	#line 24 $IDX|
	gl_FragColor = color;
}
uniform vec4 blend_color;
uniform float threshold;

//#if 0

void main(void) {
	vec4 color = texture2D(tex,gl_TexCoord[0].st);
	gl_FragColor = blend(color, blend_color);
	//gl_FragColor = makebw(color, threshold);
/*
   gl_FragColor.r = gl_TexCoord[0].x;
   gl_FragColor.g = gl_TexCoord[0].y;
   gl_FragColor.b = 0.0;
   gl_FragColor.a = 1.0;
*/
}
#endif
