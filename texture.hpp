
#pragma once

#include "gl_resource.hpp"

struct Image;

struct Texture : public gl_resource {
	static void free(unsigned int id);
	Texture(const Image& img);
	Texture();
	void set_image(const Image& img);
	void bind();
	void set_nearest();
	void set_linear();
	void set_clamp();
	void set_tile();
	void set_mirror();
};
