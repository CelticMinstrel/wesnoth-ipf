
#include <array>
#include <vector>
#include <string>
#include <map>
#include <string>

using namespace std;

using fvec2 = array<float, 2>;
using fvec3 = array<float, 3>;
using fvec4 = array<float, 4>;
using ivec2 = array<int, 2>;
using ivec3 = array<int, 3>;
using ivec4 = array<int, 4>;
using bvec2 = array<bool, 2>;
using bvec3 = array<bool, 3>;
using bvec4 = array<bool, 4>;
// Note: addressing mode is mat[col][row], or mat[x][y].
using matrix2 = array<array<float, 2>, 2>;
using matrix3 = array<array<float, 3>, 3>;
using matrix4 = array<array<float, 4>, 4>;

vector<string> split(const string& str, const string& delim);
string join(const vector<string>& elems, const string& delim);
string trim(const string& str);
string replace_all(string haystack, const map<string, string>& replacements);
string merge_strings(const vector<string>& vec);

void increment_arg_name(string& name);

fvec3 color_from_int(int c);

void check_file(const ios& file, const char* name);
string load_file(const char* name);
