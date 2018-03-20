
#include <memory>
#include <string>
#include <vector>

struct ShaderArgumentBase;
struct ShaderFunction;

using namespace std;

struct image_mod {
	string name;
	vector<shared_ptr<ShaderArgumentBase>> params;
	vector<shared_ptr<ShaderFunction>> functions;
	image_mod(const string& name) : name(name) {}
	virtual void modify_size(int& width, int& height) {}
	virtual void generate_init_code(vector<string>& code, vector<string>& files, const string& tc_param) {}
	virtual void generate_code(vector<string>& code, vector<string>& files, const string& color_param) {};
	static shared_ptr<image_mod> create(const string& code);
};
