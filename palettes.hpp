
#include <vector>
#include <map>
#include <string>

using namespace std;

struct team_color {
	int avg, max, min, mark;
};

extern const map<string, team_color> team_colors;
extern const map<string, vector<int>> palettes;