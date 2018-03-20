

all:
	clang++ -o wesnoth-ipf -g -stdlib=libc++ -std=c++11 -framework SDL2 -framework OpenGL ipf.cpp main.cpp image_mods.cpp image.cpp palettes.cpp shader.cpp texture.cpp utils.cpp