
#include "ipf.hpp"

#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>
#include <OpenGL/GL.h>
#include <OpenGL/GLU.h>

using namespace std;

#define SDL_GUARD(what) \
	do { if(what) {cerr << SDL_GetError() << endl; exit(1);} } \
	while(false)

// SDL may define main to SDL_main which can result in link errors
#ifdef main
#undef main
#endif

#if 0
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <set>

#include "image.hpp"
#include "palettes.hpp"
#include "utils.hpp"
#include "shader.hpp" // Note that this depends on some of the above headers.

struct gl_direct_context {
	gl_direct_context(int type) {glBegin(type);}
	~gl_direct_context() {glEnd();}
	bool checked = false;
	operator bool() {
		return !checked;
	}
};

#define OPENGL_RENDER(type) \
	for(gl_direct_context ctx(type); ctx; ctx.checked = true)

#define GL_SETLINE(idx) \
	("#line " + to_string(__LINE__ - 1) + " " + to_string(idx) + "\n")

static string vertex_shader;
static string fragment_shader_hdr, fragment_shader_tmpl, fragment_shader_main_tmpl;
#endif

int main(int argc, char* argv[]) {
	if(argc < 2) {
		cout << "Usage: " << argv[0] << " «ipf-string»" << endl;
		return 0;
	}
	string ipf_string(argv[1]);
	for(int i = 2; i < argc; i++)
		ipf_string += ' ' + argv[i];
	
	// Load shaders
	IPF ipf(ipf_string);
	if(!ipf.good) return -1;
	
	//ipf.show(); return 0;
	
	// Set up window, context, etc
	//Image logo("wesnoth-icon.png");
	SDL_GUARD(SDL_Init(SDL_INIT_VIDEO) < 0);
	SDL_Window* win;
	SDL_GUARD((win = SDL_CreateWindow("Wesnoth Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL)) == nullptr);
	SDL_GLContext ctx;
	SDL_GUARD((ctx = SDL_GL_CreateContext(win)) == nullptr);
	SDL_GUARD(SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1) < 0);
	
	int gl_maj, gl_min;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_min);
	cout << "Using OpenGL version " << gl_maj << '.' << gl_min << endl;
	#if 0
	GLint max_params;
	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, 0, &max_params);
	if(max_params < ipf.arguments.size()) {
		cerr << "Error: This IPF requires too many shader parameters! (needs " << ipf.arguments.size() " but we only have " << max_params << ")\n";
		return -1;
	}
	#endif
	
	//ipf.load();
	ipf.compile();
	//ipf.bind();
	
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	gluOrtho2D(0.0, 800.0, 600.0, 0.0);
	
	glClearColor(0.5, 0.5, 0, 1);
	
	SDL_Event evt;
	bool done = false;
	int frame_size = 1000.0 / 30.0;
	while(!done) {
		int ticks = SDL_GetTicks();
		while(SDL_PollEvent(&evt)) {
			switch(evt.type) {
				case SDL_QUIT:
				case SDL_KEYUP:
					done = true;
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT);
		ipf.draw(0,0);
		#if 0
		OPENGL_RENDER(GL_QUADS) {
			glTexCoord2i(0,0);
			glVertex2i(0, 0);
			glTexCoord2i(0, 1);
			glVertex2i(0,ipf.width);
			glTexCoord2i(1, 1);
			glVertex2i(ipf.height,ipf.width);
			glTexCoord2i(1, 0);
			glVertex2i(ipf.height,0);
		}
		#endif
		SDL_GL_SwapWindow(win);
		SDL_Delay(500);
	}
	
	SDL_GL_DeleteContext(ctx);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}