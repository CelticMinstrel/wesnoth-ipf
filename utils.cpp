
#include "utils.hpp"
#include <fstream>
#include <iostream>

using namespace std;

// From <https://stackoverflow.com/a/37454181/1502810>
vector<string> split(const string& str, const string& delim) {
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while(pos < str.length() && prev < str.length());
    return tokens;
}

string join(const vector<string>& elems, const string& delim) {
	if(elems.empty()) return "";
	string result = elems[0];
	for(size_t i = 1; i < elems.size(); i++)
		result += delim + elems[i];
	return result;
}

string trim(const string& str) {
	static  const string& trim_chars = " \t";
	size_t start = str.find_first_not_of(trim_chars);
	size_t end = str.find_last_not_of(trim_chars);
	return str.substr(start, end - start + 1);
}

string replace_all(string haystack, const map<string, string>& replacements) {
	for(auto& p : replacements) {
		string needle, repl;
		tie(needle, repl) = p;
		size_t start = haystack.find(needle);
		while(start != string::npos) {
			haystack = haystack.replace(start, needle.length(), repl);
			start = haystack.find(needle, start);
		}
	}
	return haystack;
}

string merge_strings(const vector<string>& vec) {
	string result;
	for(const auto& str : vec) result.append(str);
	return result;
}

void increment_arg_name(string& name) {
	if(isdigit(name.back())) {
		size_t idx = name.find_last_not_of("0123456789") + 1;
		int n = stoi(name.substr(idx));
		name.erase(idx, string::npos);
		name += to_string(n + 1);
	} else name += '1';
}

fvec3 color_from_int(int c) {
	float r = (c & 0xff0000) >> 16;
	float g = (c & 0xff00) >> 8;
	float b = c & 0xff;
	return {{r / 255.0f, g / 255.0f, b / 255.0f}};
}

void check_file(const ios& file, const char* name) {
	if(file.bad()) {
		char* err = strerror(errno);
		cerr << "Failed to load file " << name << ": " << err << '\n';
	}
}

string load_file(const char* name) {
	ifstream fin(name);
	check_file(fin, name);
	string contents;
	getline(fin, contents, '\0');
	check_file(fin, name);
	return contents;
}
