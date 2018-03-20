
#include "ipf.hpp"
#include "utils.hpp"
#include "shader.hpp"

#include <vector>
#include <iostream>
#include <set>

using namespace std;

#if 0
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <SDL2/SDL.h>
#include <OpenGL/GL.h>
#include <OpenGL/GLU.h>

using namespace std;

#include "image.hpp"
#include "palettes.hpp"
#include "utils.hpp"
#include "shader.hpp" // Note that this depends on some of the above headers.

// SDL may define main to SDL_main which can result in link errors
#ifdef main
#undef main
#endif

#define SDL_GUARD(what) \
	do { if(what) {cerr << SDL_GetError() << endl; exit(1);} } \
	while(false)

static string vertex_shader;
static string fragment_shader_hdr, fragment_shader_tmpl, fragment_shader_main_tmpl;
#endif

struct gl_direct_context {
	gl_direct_context(int type) {glBegin(type);}
	~gl_direct_context() {glEnd();}
	bool checked = false;
	operator bool() {
		return !checked;
	}
};

#define OPENGL_RENDER(type) \
	for(gl_direct_context ctx(type); ctx; ctx.checked = true)

#if 1
#include "texture.hpp"
#include "image_mods.hpp"

IPF::IPF(const string& str) {
	vector<string> tokens = split(str, "~");
	if(tokens.size() < 1) return;
	transform(tokens.begin(), tokens.end(), tokens.begin(), trim);
	base_img = Image(tokens[0].c_str());
	if(!base_img.valid) {
		cerr << "Could not load image " << tokens[0] << '\n';
		return;
	}
	width = base_img.x;
	height = base_img.y;
	tokens.erase(tokens.begin());
	for(const auto& tok : tokens) {
		shared_ptr<image_mod> next_mod = image_mod::create(tok);
		if(!next_mod) return;
		mod_queue.push_back(next_mod);
	}
	good = true;
}

void IPF::compile(bool print) {
	base.reset(new Texture(base_img));
	base->set_nearest();
	
	cout << "Compiling vertex shader...\n";
	Shader vert(load_file("shaders/vertex.glsl"), GL_VERTEX_SHADER);
	vert.show_log({"vertex.glsl"});
	
	vector<string> fragment_code{load_file("shaders/fragment-defns.glsl")}, fragment_files{"fragment-defns.glsl"};
	int i = 1;
	// Uniforms
	set<string> names;
	for(const auto& mod : mod_queue) {
		for(const auto& param : mod->params) {
			while(names.count(param->name))
				increment_arg_name(param->name);
			names.insert(param->name);
			fragment_code.push_back(GL_SETLINE(i++) + replace_all("uniform $TYPE| $NAME|;\n", {
				{"$TYPE|", param->type()},
				{"$NAME|", param->name},
			}));
			fragment_files.push_back(__FILE__ "~" + mod->name + "~U");
		}
		mod->modify_size(width, height);
	}
	// Functions
	for(const auto& mod : mod_queue) {
		for(const auto& fcn : mod->functions) {
			while(names.count(fcn->name))
				increment_arg_name(fcn->name);
			names.insert(fcn->name);
			fragment_code.push_back(GL_SETLINE(i++) + replace_all("\n"
"$RESULT| $NAME|($PARAMS|) {\n"
"$CODE|\n"
"}\n",
			{
				{"$RESULT|", fcn->result},
				{"$NAME|", fcn->name},
				{"$PARAMS|", join(fcn->params, ",")},
				{"$CODE|", join(fcn->code, "\n")},
			}));
			fragment_files.push_back(__FILE__ "~" + mod->name + "~F");
		}
	}
	set<string> local_names = {"tc", "color"};
	fragment_code.push_back(GL_SETLINE(i) + "\nvoid main(void) {\nvec3 tc = vec3(gl_TexCoord[0].st, 1);\n");
	fragment_files.push_back(__FILE__);
	// Init Code
	for(const auto& mod : mod_queue)
		mod->generate_init_code(fragment_code, fragment_files, "tc");
	fragment_code.push_back(GL_SETLINE(i) + "vec4 color = texture2D(base_tex, tc.st);\n");
	// Execution Code
	for(const auto& mod : mod_queue)
		mod->generate_code(fragment_code, fragment_files, "color");
	fragment_code.push_back(GL_SETLINE(i) + "\n\tgl_FragColor = color;\n}\n");
	
	cout << "Compiling fragment shader...\n";
	Shader frag(fragment_code, GL_FRAGMENT_SHADER);
	frag.show_log(fragment_files);
	
	if(!vert.good || !frag.good) good = false;
	prog.reset(new ShaderProgram(vert, frag));
	if(!prog->good) good = false;
	prog->show_log();
	
	if(print) {
		cout << "Fragment shader:\n";
		fragment_code.erase(fragment_code.begin());
		for(const string& segment : fragment_code)
			cout << segment;
		cout << endl;
	}
	
	if(!good) exit(-1);
}

// TODO: Replace this with some sort of get_vertices call?
void IPF::draw(int x, int y) {
	if(base) base->bind();
	if(prog) {
		glUseProgram(prog->id);
		for(const auto& mod : mod_queue)
			for(const auto& arg : mod->params)
				arg->apply(*prog);
	}
	OPENGL_RENDER(GL_QUADS) {
		glTexCoord2i(0, 0); glVertex2i(x, y);
		glTexCoord2i(0, 1); glVertex2i(x, y + height);
		glTexCoord2i(1, 1); glVertex2i(x + width, y + height);
		glTexCoord2i(1, 0); glVertex2i(x + width, y);
	}
}

#else
struct IPF {
	vector<string> params, color_mutations, tex_coord_mutations;
	map<string,string> fragment_ops;
	vector<string> filenames_vert, filenames_frag;
	vector<Image> textures;
	vector<GLuint> texture_ids;
	vector<shared_ptr<ShaderArgumentBase>> arguments;
	shared_ptr<ShaderProgram> prog;
	int width, height;
	bool good = false;
	IPF(const string& str, const string& id = "base");
	void show() {
		cout << "Vertex shader:\n";
		for(const auto& str : build_vert()) cout << str << '\n';
		cout << "\nFragment shader:\n";
		for(const auto& str : build_frag()) cout << str << '\n';
	}
	vector<string> build_vert() {
		return {vertex_shader};
	}
	vector<string> build_frag() {
		vector<string> frag;
		
		filenames_frag.push_back("fragment-defns.glsl");
		frag.push_back(fragment_shader_hdr);
		copy(params.begin(), params.end(), back_inserter(frag));
		
		vector<string> fcn_calls;
		fcn_calls.reserve(fragment_ops.size());
		int i = 1;
		for(const auto& fcn : fragment_ops) {
			filenames_frag.push_back("fragment-fcn.glsl~" + fcn.first);
			string fragment_fcn = fragment_shader_tmpl;
			fragment_fcn = replace_first(fragment_fcn, "$FCN|", fcn.first);
			fragment_fcn = replace_all(fragment_fcn, "$IDX|", to_string(i));
			frag.push_back(fragment_fcn);
			fcn_calls.push_back(fcn.second);
			i++;
		}
		
		filenames_frag.push_back("fragment-main.glsl");
		string fragment_main = fragment_shader_main_tmpl;
		fragment_main = replace_first(fragment_main, "$FUNCTION_CALLS|", merge_strings(fcn_calls));
		fragment_main = replace_first(fragment_main, "$COLOR_MUTATIONS|", merge_strings(color_mutations));
		fragment_main = replace_first(fragment_main, "$TEX_COORD_MUTATIONS|", merge_strings(tex_coord_mutations));
		fragment_main = replace_all(fragment_main, "$IDX|", to_string(i));
		frag.push_back(fragment_main);
		
		return frag;
	}
	void compile() {
		for(const auto& arg : arguments)
			arg->prep(*this);
		
		auto vert = build_vert(), frag = build_frag();
		
		cout << "Compiling vertex shader...\n";
		Shader vert_shader(vert, GL_VERTEX_SHADER);
		vert_shader.show_log(filenames_vert);
		cout << "Compiling fragment shader...\n";
		Shader frag_shader(frag, GL_FRAGMENT_SHADER);
		frag_shader.show_log(filenames_frag);
		if(!vert_shader.good || !frag_shader.good) good = false;
		prog.reset(new ShaderProgram(vert_shader, frag_shader));
		if(!prog->good) good = false;
		prog->show_log();
		if(!good) exit(-1);
	}
	void load() {
		texture_ids.resize(textures.size());
		glGenTextures(texture_ids.size(), texture_ids.data());
		for(int i = 0; i < texture_ids.size(); i++) {
			glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[i].x, textures[i].y, 0, GL_RGBA, GL_UNSIGNED_BYTE, textures[i].data);
		}
	}
	void bind() {
		glUseProgram(prog->id);
		for(const auto& arg : arguments)
			arg->apply(*prog);
	}
};

template<> struct ShaderArgument<string> : public ShaderArgumentBase {
	IPF sub_ipf;
	int texture_id;
	ShaderArgument(const string& name, const string& val) : ShaderArgumentBase(name), sub_ipf(val) {}
	void merge_arrays(const vector<string>& from, vector<string>& into, size_t at = 0) {
		into.insert(into.begin() + at, from.begin(), from.end());
	}
	void prep(IPF& owner) override {
		// Here we need to merge our shader code into the owner's shader code.
		// If you think of this shader as an independent entity which produces a result, the purpose of this function is to integrate that result into the flow of the main program.
		// This means we need to:
		// 1. Ensure there are no name conflicts between the two entities.
		// 2. Ensure any sub-ipfs are also resolved.
		// 3. Add our code to the owner's lists.
		for(auto& arg : sub_ipf.arguments)
			arg->prep(owner);
		merge_arrays(sub_ipf.params, owner.params);
		merge_arrays(sub_ipf.tex_coord_mutations, owner.tex_coord_mutations);
		merge_arrays(sub_ipf.params, owner.params);
		merge_arrays(sub_ipf.color_mutations, owner.color_mutations);
		//merge_arrays(sub_ipf.textures, owner.textures);
		owner.fragment_ops.insert(sub_ipf.fragment_ops.begin(), sub_ipf.fragment_ops.end());
		merge_arrays(sub_ipf.filenames_vert, owner.filenames_vert, 1);
		merge_arrays(sub_ipf.filenames_frag, owner.filenames_frag, 1);
	}
	void apply(ShaderProgram& prog) const override {
		prog.setUniform(name, texture_id);
	}
};

IPF::IPF(const string& str, const string& id) {
	vector<string> tokens = split(str, "~");
	if(tokens.size() < 1) return;
	
	if(id != "base") {
		for(auto& p : next_arg_name)
			p.second = id + '_' + p.second;
	}
	
	//fragment_ops.emplace(id, replace_first(replace_first("color = $FCN|($TEXTURE|, gl_TexCoord[0].st);", "$FCN|", id), "$TEXTURE|", id + "_tex"));
	{
		Image base_tex(tokens[0].c_str());
		if(!base_tex.valid) return;
		width = base_tex.x;
		height = base_tex.y;
		textures.push_back(move(base_tex));
	}
	tokens.erase(tokens.begin());
	for(const auto& tok : tokens) {
		size_t beg = tok.find_first_of('(');
		size_t end = tok.find_last_of(')');
		size_t args_len = end - beg - 1;
		if(end != tok.size() - 1) {
			cerr << "Extra unknown tokens after IPF: " << tok.substr(end + 1) << '\n';
			return;
		}
		string name = tok.substr(0, beg);
		string& argname = next_arg_name[name];
		if(name == "L") {
			// TODO: Support IPFs tacked onto the lightmap, too
			params.push_back(replace_first("uniform sampler2D $ARG|;\n", "$ARG|", argname));
			color_mutations.push_back(replace_all("color = light(color, texture2D($ARG|, gl_TexCoord[0].st));\n", "$ARG|", argname));
			auto arg = make_argument(argname, tok.substr(beg + 1, args_len));
			
			arguments.push_back(arg);
		}
		increment_arg_name(argname);
	}
	
	good = true;
}
#endif
