
#include "shader.hpp"

#include <array>
#include <vector>
#include <string>
#include <type_traits>
#include <sstream>
#include <iostream>

#include "gl_resource.hpp"

using namespace std;

Shader::Shader(const string& code, int type)
	: gl_resource(glCreateShader(type), glDeleteShader)
{
	const char* src = code.c_str();
	glShaderSource(id, 1, &src, nullptr);
	finishCompile();
}

Shader::Shader(const vector<string>& code, int type)
	: gl_resource(glCreateShader(type), glDeleteShader)
{
	vector<const char*> vec;
	transform(code.begin(), code.end(), back_inserter(vec), [](const string& s){return s.c_str();});
	glShaderSource(id, code.size(), vec.data(), nullptr);
	finishCompile();
}

void Shader::show_log(const vector<string>& files) {
	// Check for errors or warnings
	int logLength;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0) {
		vector<char> shaderError(logLength);
		glGetShaderInfoLog(id, logLength, nullptr, shaderError.data());
		istringstream err_in(shaderError.data());
		string line;
		while(getline(err_in, line)) {
			size_t beg = line.find_first_of("0123456789");
			size_t end = line.find_first_not_of("0123456789", beg + 1);
			cout << line.substr(0, beg);
			int n = stoi(line.substr(beg, end - beg));
			if(n < files.size()) cout << files[n];
			// For some reason trigraphs are enabled, so escape the third question mark...
			else cout << "<??\?>";
			cout << line.substr(end) << endl;
		}
	}
}

void Shader::finishCompile() {
	glCompileShader(id);
	// Check for errors
	int succeeded;
	glGetShaderiv(id, GL_COMPILE_STATUS, &succeeded);
	good = succeeded;
}

ShaderProgram::ShaderProgram(const Shader& vert, const Shader& frag)
	: gl_resource(glCreateProgram(), glDeleteProgram)
	, vert(vert)
	, frag(frag)
{
	glAttachShader(id, vert.id);
	glAttachShader(id, frag.id);
	glLinkProgram(id);
	// Check for errors or warnings
	int succeeded;
	glGetProgramiv(id, GL_LINK_STATUS, &succeeded);
	good = succeeded;
}

void ShaderProgram::show_log() {
	// Check for errors or warnings
	int logLength;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0) {
		vector<char> programError(logLength);
		glGetProgramInfoLog(id, logLength, nullptr, programError.data());
		cout << programError.data() << endl;
	}
}

ShaderArgumentBase::ShaderArgumentBase(const string& name) : name(name) {}

template<> const string ShaderType<float>::name = "float";
template<> const string ShaderType<fvec2>::name = "vec2";
template<> const string ShaderType<fvec3>::name = "vec3";
template<> const string ShaderType<fvec4>::name = "vec4";
template<> const string ShaderType<int>::name = "int";
template<> const string ShaderType<ivec2>::name = "ivec2";
template<> const string ShaderType<ivec3>::name = "ivec3";
template<> const string ShaderType<ivec4>::name = "ivec4";
template<> const string ShaderType<bool>::name = "bool";
template<> const string ShaderType<bvec2>::name = "bvec2";
template<> const string ShaderType<bvec3>::name = "bvec3";
template<> const string ShaderType<bvec4>::name = "bvec4";
template<> const string ShaderType<matrix2>::name = "mat2";
template<> const string ShaderType<matrix3>::name = "mat3";
template<> const string ShaderType<matrix4>::name = "mat4";
template<> const string ShaderType<team_color>::name = "team_color";

//template<> const string ShaderType<string>::name = "sampler2D";
