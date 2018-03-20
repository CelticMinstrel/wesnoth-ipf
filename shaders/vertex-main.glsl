
#line 2 1

void main(void) {
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 tc = gl_MultiTexCoord0;
	$TEX_COORD_MUTATIONS|
	#line 8 1
	gl_TexCoord[0] = tc;
}

#if 0
void main(void) {
   vec4 a = gl_Vertex;

   gl_Position = gl_ModelViewProjectionMatrix * a;
   gl_TexCoord[0] = gl_MultiTexCoord0;
   gl_TexCoord[0].stp *= flip(false, true);
}
#endif
