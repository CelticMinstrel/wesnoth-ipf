
#include "image.hpp"
#include <iostream>

#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

Image::Image() : valid(false) {}

Image::Image(const char* fname) {
	data = stbi_load(fname, &x, &y, &comp, 4);
	if(!data) {
		valid = false;
		cerr << stbi_failure_reason() << endl;
	}
}

Image::Image(Image&& other) : x(other.x), y(other.y), comp(other.comp), data(other.data), valid(other.valid) {
	other.data = nullptr;
	other.valid = false;
}

Image& Image::operator=(Image&& other) {
	x = other.x;
	y = other.y;
	comp = other.comp;
	data = other.data;
	valid = other.valid;
	other.data = nullptr;
	other.valid = false;
	return *this;
}

Image::~Image() {
	if(data) stbi_image_free(data);
}
