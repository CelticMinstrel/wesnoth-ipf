
#pragma once

#include "gl_resource.hpp"
#include "utils.hpp"

#include <array>
#include <vector>
#include <string>
#include <type_traits>
#include <sstream>
#include <OpenGL/GL.h>
#include "palettes.hpp"

#define GL_SETLINE(idx) \
	("#line " + to_string(__LINE__ - 1) + " " + to_string(idx) + "\n")

struct Shader : public gl_resource {
	bool good = false;
	// Type is GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
	Shader(const string& code, int type);
	Shader(const vector<string>& code, int type);
	void show_log(const vector<string>& files);
private:
	void finishCompile();
};

struct ShaderProgram : public gl_resource {
	Shader vert, frag;
	bool good = false;
	ShaderProgram(const Shader& vert, const Shader& frag);
	void show_log();
	template<typename T>
	void setUniform(const string& name, const T& value);
	template<typename T>
	void setAttrib(const string& name, const T& value);
private:
	template<typename Fcn, typename... Params>
	void setUniformImpl(const string& name, Fcn setter, Params... args) {
		int where = glGetUniformLocation(id, name.c_str());
		setter(where, args...);
	}
	template<typename Fcn, typename... Params>
	void setAttribImpl(const string& name, Fcn setter, Params... args) {
		int where = glGetAttribLocation(id, name.c_str());
		setter(where, args...);
	}
};

template<typename T>
using safe_vector_of = vector<typename conditional<is_same<T, bool>::value, int, T>::type>;

template<typename T, size_t n>
inline safe_vector_of<T> flattenVecArray(const vector<array<T,n>>& arr) {
	safe_vector_of<T> vec;
	vec.reserve(arr.size() * n);
	for(auto val : arr)
		for(auto comp : val)
			vec.push_back(comp);
	return vec;
}

template<typename T, size_t n>
inline vector<T> flattenMatrix(const array<array<T, n>, n> mat) {
	vector<T> vec;
	vec.reserve(n * n);
	for(auto col : mat)
		for(auto elem : col)
			vec.push_back(elem);
	return vec;
}

template<typename T, size_t n>
inline vector<T> flattenMatrixArray(const vector<array<array<T, n>, n>> arr) {
	vector<T> vec;
	vec.reserve(arr.size() * n * n);
	for(auto mat : arr)
		for(auto col : mat)
			for(auto elem : col)
				vec.push_back(elem);
	return vec;
}

// Now follows the many, many setUniform specializations...
// 1. floats, float vectors, and float arrays.
template<> inline void ShaderProgram::setUniform(const string& name, const float& val) {
	setUniformImpl(name, glUniform1f, val);
}

template<> inline void ShaderProgram::setUniform(const string& name, const fvec2& val) {
	setUniformImpl(name, glUniform2f, val[0], val[1]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const fvec3& val) {
	setUniformImpl(name, glUniform3f, val[0], val[1], val[2]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const fvec4& val) {
	setUniformImpl(name, glUniform4f, val[0], val[1], val[2], val[3]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<float>& val) {
	setUniformImpl(name, glUniform1fv, val.size(), val.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<fvec2>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform2fv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<fvec3>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform3fv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<fvec4>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform4fv, val.size(), vec.data());
}

// 2. ints, int vectors, and int arrays
// NB: These are also used for sampler parameters, since texture IDs are ints.
template<> inline void ShaderProgram::setUniform(const string& name, const int& val) {
	setUniformImpl(name, glUniform1i, val);
}

template<> inline void ShaderProgram::setUniform(const string& name, const ivec2& val) {
	setUniformImpl(name, glUniform2i, val[0], val[1]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const ivec3& val) {
	setUniformImpl(name, glUniform3i, val[0], val[1], val[2]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const ivec4& val) {
	setUniformImpl(name, glUniform4i, val[0], val[1], val[2], val[3]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<int>& val) {
	setUniformImpl(name, glUniform1iv, val.size(), val.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<ivec2>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform2iv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<ivec3>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform3iv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<ivec4>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform4iv, val.size(), vec.data());
}

// 3. bools, bool vectors, and bool arrays
template<> inline void ShaderProgram::setUniform(const string& name, const bool& val) {
	setUniformImpl(name, glUniform1i, val);
}

template<> inline void ShaderProgram::setUniform(const string& name, const bvec2& val) {
	setUniformImpl(name, glUniform2i, val[0], val[1]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const bvec3& val) {
	setUniformImpl(name, glUniform3i, val[0], val[1], val[2]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const bvec4& val) {
	setUniformImpl(name, glUniform4i, val[0], val[1], val[2], val[3]);
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<bool>& val) {
	vector<int> vec(val.begin(), val.end());
	setUniformImpl(name, glUniform1iv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<bvec2>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform2iv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<bvec3>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform3iv, val.size(), vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<bvec4>& val) {
	auto vec = flattenVecArray(val);
	setUniformImpl(name, glUniform4iv, val.size(), vec.data());
}

// 4. matrices and matrix arrays
template<> inline void ShaderProgram::setUniform(const string& name, const matrix2& val) {
	auto vec = flattenMatrix(val);
	setUniformImpl(name, glUniformMatrix2fv, 1, false, vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const matrix3& val) {
	auto vec = flattenMatrix(val);
	setUniformImpl(name, glUniformMatrix2fv, 1, false, vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const matrix4& val) {
	auto vec = flattenMatrix(val);
	setUniformImpl(name, glUniformMatrix2fv, 1, false, vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<matrix2>& val) {
	auto vec = flattenMatrixArray(val);
	setUniformImpl(name, glUniformMatrix2fv, val.size(), false, vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<matrix3>& val) {
	auto vec = flattenMatrixArray(val);
	setUniformImpl(name, glUniformMatrix2fv, val.size(), false, vec.data());
}

template<> inline void ShaderProgram::setUniform(const string& name, const vector<matrix4>& val) {
	auto vec = flattenMatrixArray(val);
	setUniformImpl(name, glUniformMatrix2fv, val.size(), false, vec.data());
}

// 5. The specializations of setAttrib, which are far fewer in number.
template<> inline void ShaderProgram::setAttrib(const string& name, const float& val) {
	setAttribImpl(name, glVertexAttrib1f, val);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const fvec2& val) {
	setAttribImpl(name, glVertexAttrib2f, val[0], val[1]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const fvec3& val) {
	setAttribImpl(name, glVertexAttrib3f, val[0], val[1], val[2]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const fvec4& val) {
	setAttribImpl(name, glVertexAttrib4f, val[0], val[1], val[2], val[3]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const int& val) {
	setAttribImpl(name, glVertexAttrib1s, val);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const ivec2& val) {
	setAttribImpl(name, glVertexAttrib2s, val[0], val[1]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const ivec3& val) {
	setAttribImpl(name, glVertexAttrib3s, val[0], val[1], val[2]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const ivec4& val) {
	setAttribImpl(name, glVertexAttrib4s, val[0], val[1], val[2], val[3]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const bool& val) {
	setAttribImpl(name, glVertexAttrib1s, val);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const bvec2& val) {
	setAttribImpl(name, glVertexAttrib2s, val[0], val[1]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const bvec3& val) {
	setAttribImpl(name, glVertexAttrib3s, val[0], val[1], val[2]);
}

template<> inline void ShaderProgram::setAttrib(const string& name, const bvec4& val) {
	setAttribImpl(name, glVertexAttrib4s, val[0], val[1], val[2], val[3]);
}

struct ShaderArgumentBase {
	string name;
	ShaderArgumentBase(const string& name);
	virtual void apply(ShaderProgram& prog) const = 0;
	//virtual void prep(struct IPF& owner) {}
	virtual string type() const = 0;
};

template<typename T>
struct ShaderType {
	static const string name;
};

template<typename T>
struct ShaderArgument : public ShaderArgumentBase {
	using base_type = typename remove_cv<typename remove_reference<T>::type>::type;
	base_type& value;
	ShaderArgument(const string& name, base_type& val) : ShaderArgumentBase(name), value(val) {}
	void apply(ShaderProgram& prog) const override {
		prog.setUniform(name, value);
	}
	string type() const override {
		return ShaderType<base_type>::name;
	}
};

template<>
inline void ShaderArgument<team_color&>::apply(ShaderProgram& prog) const {
	fvec3 avg, min, max;
	avg = color_from_int(value.avg);
	min = color_from_int(value.min);
	max = color_from_int(value.max);
	string qualified_name = name + '.';
	int pos = qualified_name.size();
	qualified_name += "mid";
	prog.setUniform(qualified_name, color_from_int(value.avg));
	qualified_name.replace(pos, 3, "min");
	prog.setUniform(qualified_name, color_from_int(value.min));
	qualified_name.replace(pos, 3, "max");
	prog.setUniform(qualified_name, color_from_int(value.max));
}

template<typename T>
struct ShaderArgument<vector<T>&> : public ShaderArgumentBase {
	using value_type = typename remove_cv<typename remove_reference<T>::type>::type;
	using base_type = vector<value_type>;
	base_type& value;
	size_t max_size;
	ShaderArgument(const string& name, base_type& val, size_t max_size) : ShaderArgumentBase(name), value(val), max_size(max_size) {}
	void apply(ShaderProgram& prog) const override {
		prog.setUniform(name, value);
	}
	string type() const override {
		return ShaderType<value_type>::name + "[" + to_string(max_size) + "]";
	}
};

template<typename T>
inline shared_ptr<ShaderArgument<T>> make_argument(const string& name, T&& val) {
	return shared_ptr<ShaderArgument<T>>(new ShaderArgument<T>(name, std::forward<T>(val)));
}

template<typename T>
inline shared_ptr<ShaderArgument<vector<T>&>> make_argument(const string& name, vector<T>& val, size_t max_size) {
	return shared_ptr<ShaderArgument<vector<T>&>>(new ShaderArgument<vector<T>&>(name, val, max_size));
}

//template<typename Fcn>
struct ShaderFunction {
	string name, result;
	vector<string> params, code;
};
