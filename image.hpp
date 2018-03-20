
#include <memory>
#include <vector>

using namespace std;

struct Image {
	int x, y, comp;
	unsigned char* data;
	bool valid = true;
	Image();
	Image(const char* fname);
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&& other);
	Image& operator=(Image&& other);
	~Image();
};