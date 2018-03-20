
#include "image_mods.hpp"
#include "ipf.hpp"
#include "utils.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <iostream>
#include <cmath>

struct fl_mod : public image_mod {
	bvec2 flip_dir;
	fl_mod(const vector<string>& args) : image_mod("FL") {
		if(args.size() > 1)
			throw string("Too many arguments to FL");
		if(args.size() == 0) {
			flip_dir = {true, false};
		} else {
			flip_dir[0] = args[0].find("horiz") != string::npos;
			flip_dir[1] = args[0].find("vert") != string::npos;
		}
		params.push_back(make_argument("flip_dir", flip_dir));
	}
	void generate_init_code(vector<string>& code, vector<string>& files, const string& tc_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$TEX_COORDS| *= flip($ARG|.x, $ARG|.y);\n", {
			{"$TEX_COORDS|", tc_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~FL");
	}
};

struct rotate_mod : public image_mod {
	float angle;
	rotate_mod(const vector<string>& args) : image_mod("ROTATE") {
		if(args.size() > 1)
			throw string("Too many arguments to ROTATE");
		if(args.size() == 0) angle = 90;
		else try {
			angle = stof(args[0]);
		} catch(invalid_argument& x) {
			throw string("Bad angle for ROTATE");
		}
		params.push_back(make_argument("angle", angle));
	}
	void modify_size(int& width, int& height) override {
		float theta = fmod(angle, 360);
		if(theta == 90 || theta == 270) {
			swap(width, height);
		} else if(theta != 0 && theta != 180) {
			// TODO: Arbitrary rotations
		}
	}
	void generate_init_code(vector<string>& code, vector<string>& files, const string& tc_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$TEX_COORDS| *= rotate(vec2(0.0,0.0), $ARG|);\n", {
			{"$TEX_COORDS|", tc_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~ROTATE");
	}
};

struct scale_mod : public image_mod {
	int new_width, new_height;
	scale_mod(const vector<string>& args) : image_mod("SCALE") {
		if(args.size() != 2)
			throw string("Wrong number of arguments to SCALE");
		try {
			new_width = stoi(args[0]);
			new_height = stoi(args[1]);
		} catch(invalid_argument& x) {
			throw string("Bad argument to SCALE");
		}
	}
	void modify_size(int& width, int& height) override {
		width = new_width;
		height = new_height;
	}
};

struct scale_into_mod : public image_mod {
	int frame_width, frame_height;
	scale_into_mod(const vector<string>& args) : image_mod("SCALE_INTO") {
		if(args.size() != 2)
			throw string("Wrong number of arguments to SCALE_INTO");
		try {
			frame_width = stoi(args[0]);
			frame_height = stoi(args[1]);
		} catch(invalid_argument& x) {
			throw string("Bad argument to SCALE_INTO");
		}
		if(frame_width < 0 || frame_height < 0)
			throw string("Bad argument to SCALE_INTO");
	}
	void modify_size(int& width, int& height) override {
		long double w = frame_width;
		long double h = frame_height;
		
		long double ratio = std::min(w / width, h / height);
		width *= ratio;
		height *= ratio;
	}
};

struct blend_mod : public image_mod {
	fvec4 blend_color;
	blend_mod(const vector<string>& args) : image_mod("BLEND") {
		if(args.size() != 4)
			throw string("Wrong number of arguments to BLEND: " + to_string(args.size()));
		try {
			transform(args.begin(), args.end() - 1, blend_color.begin(), [](const string& s){
				return stoi(s) / 255.0f;
			});
			if(args[3].back() == '%')
				blend_color[3] = stoi(args[3].substr(0, args[3].size() - 1)) / 100.0f;
			else blend_color[3] = stof(args[3]);
		} catch(invalid_argument& x) {
			throw string("Bad argument to BLEND");
		}
		params.push_back(make_argument("blend_color", blend_color));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = blend($COLOR|, $ARG|);\n", {
			{"$COLOR|", color_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~BLEND");
	}
};

struct gs_mod : public image_mod {
	gs_mod(const vector<string>& args) : image_mod("GS") {
		if(!args.empty())
			throw string("GS takes no arguments");
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = greyscale($COLOR|);\n", {
			{"$COLOR|", color_param},
		}));
		files.push_back(__FILE__ "~GS");
	}
};

struct bw_mod : public image_mod {
	float threshold;
	bw_mod(const vector<string>& args) : image_mod("BW") {
		if(args.size() != 1)
			throw string("Wrong number of arguments to BW");
		try {
			threshold = stof(args[0]);
		} catch(invalid_argument& x) {
			throw string("Bad argument to BW");
		}
		params.push_back(make_argument("threshold", threshold));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = monochrome($COLOR|, $ARG|);\n", {
			{"$COLOR|", color_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~BW");
	}
};

struct pal_mod : public image_mod {
	vector<fvec3> source_pal, dest_pal;
	int pal_size;
	pal_mod(const vector<string>& args) : image_mod("PAL") {
		if(args.size() != 1)
			throw string("Wrong number of arguments to PAL");
		vector<string> colors = split(args[0], ">");
		// Now do the stuff that's implicit to Wesnoth's split...
		transform(colors.begin(), colors.end(), colors.begin(), trim);
		colors.erase(remove_if(colors.begin(), colors.end(), bind(&vector<string>::empty, &colors)), colors.end());
		if(colors.size() != 2)
			throw string("Wrong number of arguments to PAL");
		auto src_iter = palettes.find(colors[0]);
		auto dst_iter = palettes.find(colors[1]);
		if(src_iter == palettes.end())
			throw string("Invalid source palette for PAL: " + colors[0]);
		if(dst_iter == palettes.end())
			throw string("Invalid dest palette for PAL: " + colors[1]);
		pal_size = std::min(src_iter->second.size(), dst_iter->second.size());
		pal_size = std::min(pal_size, 256);
		source_pal.reserve(pal_size);
		dest_pal.reserve(pal_size);
		transform(src_iter->second.begin(), src_iter->second.begin() + pal_size, back_inserter(source_pal), color_from_int);
		transform(dst_iter->second.begin(), dst_iter->second.begin() + pal_size, back_inserter(dest_pal), color_from_int);
		params.push_back(make_argument("palette_src", source_pal, 256));
		params.push_back(make_argument("palette_dst", dest_pal, 256));
		params.push_back(make_argument("palette_sz", pal_size));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = recolor($SRC|, $DST|, $COLOR|, $LEN|);\n", {
			{"$COLOR|", color_param},
			{"$SRC|", params[0]->name},
			{"$DST|", params[1]->name},
			{"$LEN|", params[2]->name},
		}));
		files.push_back(__FILE__ "~PAL");
	}
};

struct rc_mod : public image_mod {
	vector<fvec3> source_pal;
	team_color dest_range;
	int pal_size;
	rc_mod(const vector<string>& args) : image_mod("RC") {
		if(args.size() != 1)
			throw string("Wrong number of arguments to RC");
		vector<string> colors = split(args[0], ">");
		// Now do the stuff that's implicit to Wesnoth's split...
		transform(colors.begin(), colors.end(), colors.begin(), trim);
		colors.erase(remove_if(colors.begin(), colors.end(), bind(&vector<string>::empty, &colors)), colors.end());
		if(colors.size() != 2)
			throw string("Wrong number of arguments to RC");
		auto src_iter = palettes.find(colors[0]);
		auto dst_iter = team_colors.find(colors[1]);
		if(src_iter == palettes.end())
			throw string("Invalid source palette for RC: " + colors[0]);
		if(dst_iter == team_colors.end())
			throw string("Invalid dest range for RC: " + colors[1]);
		pal_size = std::min<int>(src_iter->second.size(), 256);
		source_pal.reserve(pal_size);
		dest_range = dst_iter->second;
		transform(src_iter->second.begin(), src_iter->second.begin() + pal_size, back_inserter(source_pal), color_from_int);
		params.push_back(make_argument("rc_palette", source_pal, 256));
		params.push_back(make_argument("rc_range", dest_range));
		params.push_back(make_argument("rc_palsize", pal_size));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = recolor($SRC|, $DST|, $COLOR|, $LEN|);\n", {
			{"$COLOR|", color_param},
			{"$SRC|", params[0]->name},
			{"$DST|", params[1]->name},
			{"$LEN|", params[2]->name},
		}));
		files.push_back(__FILE__ "~PAL");
	}
};

template<typename T>
struct cs_mod_base : public image_mod {
	fvec3 shift;
	cs_mod_base(const vector<string>& args, const string& name) : image_mod(name) {
		shift = static_cast<T*>(this)->parse_args(args);
		params.push_back(make_argument("shift", shift));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = blend_add($COLOR|, vec4($ARG|, 0));\n", {
			{"$COLOR|", color_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~CS");
	}
};

struct cs_mod : public cs_mod_base<cs_mod> {
	cs_mod(const vector<string>& args) : cs_mod_base(args, "CS") {}
	static fvec3 parse_args(const vector<string>& args) {
		if(args.size() > 3)
			throw string("Too many arguments to CS: " + to_string(args.size()));
		fvec3 shift;
		transform(args.begin(), args.end(), shift.begin(), [](const string& s) {return stoi(s) / 255.0;});
		return shift;
	}
};

static const char* cs_mods[] = {"R", "G", "B"};
template<int i>
struct cs_mod_single : public cs_mod_base<cs_mod_single<i>> {
	cs_mod_single(const vector<string>& args) : cs_mod_base<cs_mod_single<i>>(args, cs_mods[i]) {}
	static fvec3 parse_args(const vector<string>& args) {
		if(args.size() != 1)
			throw string("Wrong number of arguments to ") + cs_mods[i];
		fvec3 shift = {{0,0,0}};
		try {
			shift[i] = stoi(args[0]) / 255.0;
		} catch(invalid_argument& x) {
			throw string("Bad argument to ") + cs_mods[i];
		}
		return shift;
	}
};

using r_mod = cs_mod_single<0>;
using g_mod = cs_mod_single<1>;
using b_mod = cs_mod_single<2>;

struct neg_mod : public image_mod {
	fvec3 threshold = {{-1, -1, -1}};
	neg_mod(const vector<string>& args) : image_mod("NEG") {
		if(args.size() == 2 || args.size() > 3)
			throw string("Wrong number of arguments to NEG");
		try {
			if(args.size() == 3) {
				for(int i = 0; i < args.size(); i++) {
					int val = stoi(args[i]);
					if(val >= 0)
						threshold[i] = val / 255.0f;
				}
			} else if(args.size() == 1)
				fill(threshold.begin(), threshold.end(), stoi(args[0]) / 255.0f);
		} catch(invalid_argument& x) {
			throw string("Bad argument to NEG");
		}
		params.push_back(make_argument("threshold", threshold));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = invert($COLOR|, $ARG|);\n", {
			{"$COLOR|", color_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~NEG");
	}
};

struct swap_mod : public image_mod {
	string swizzle;
	swap_mod(const vector<string>& args) : image_mod("SWAP") {
		static const map<string, char> channel_names = {
			{"red", 'r'},
			{"green", 'g'},
			{"blue", 'b'},
			{"alpha", 'a'},
		};
		if(args.size() < 3 || args.size() > 4)
			throw string("Wrong number of arguments to SWAP: ") + to_string(args.size());
		for(const auto& arg : args) {
			auto iter = channel_names.find(arg);
			if(iter == channel_names.end())
				throw string("Invalid argument to SWAP: ") + arg;
			swizzle += iter->second;
		}
		if(swizzle.size() == 3) swizzle += 'a';
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = $COLOR|.$CHANNELS|;\n", {
			{"$COLOR|", color_param},
			{"$CHANNELS|", swizzle},
		}));
		files.push_back(__FILE__ "~SWAP");
	}
};

struct plot_alpha_mod : public image_mod {
	plot_alpha_mod(const vector<string>& args) : image_mod("PLOT_ALPHA") {
		if(!args.empty()) throw string("PLOT_ALPHA does not take arguments");
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR|.rgb = $COLOR|.aaa;\n$COLOR|.a = 1;\n", {
			{"$COLOR|", color_param},
		}));
		files.push_back(__FILE__ "~PLOT_ALPHA");
	}
};

struct wipe_alpha_mod : public image_mod {
	wipe_alpha_mod(const vector<string>& args) : image_mod("WIPE_ALPHA") {
		if(!args.empty()) throw string("WIPE_ALPHA does not take arguments");
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR|.a = 1;\n", {
			{"$COLOR|", color_param},
		}));
		files.push_back(__FILE__ "~WIPE_ALPHA");
	}
};

struct sepia_mod : public image_mod {
	sepia_mod(const vector<string>& args) : image_mod("SEPIA") {
		if(!args.empty()) throw string("SEPIA does not take arguments");
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = sepia($COLOR|);\n", {
			{"$COLOR|", color_param},
		}));
		files.push_back(__FILE__ "~SEPIA");
	}
};

struct o_mod : public image_mod {
	float opacity;
	o_mod(const vector<string>& args) : image_mod("O") {
		if(args.size() != 1)
			throw string("Wrong number of arguments to O");
		try {
			if(args[0].back() == '%')
				opacity = stoi(args[0].substr(0, args[0].size() - 1)) / 100.0f;
			else opacity = stof(args[0]);
		} catch(invalid_argument&) {
			throw string("Invalid argument to O");
		}
		params.push_back(make_argument("opacity", opacity));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) override {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR|.a *= $ARG|;\n", {
			{"$COLOR|", color_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~O");
	}
};

struct bg_mod : public image_mod {
	fvec3 bg_color;
	bg_mod(const vector<string>& args) : image_mod("BG") {
		if(args.size() != 3)
			throw string("Wrong number of arguments to BG");
		try {
			transform(args.begin(), args.end(), bg_color.begin(), [](const string& s){return stoi(s) / 255.0f;});
		} catch(invalid_argument& x) {
			throw string("Bad integer argument to BG\n");
		}
		params.push_back(make_argument("bg_color", bg_color));
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = blend_alpha(vec4($ARG|, 1.0), $COLOR|);\n", {
			{"$COLOR|", color_param},
			{"$ARG|", params[0]->name},
		}));
		files.push_back(__FILE__ "~BG");
	}
};

#if 0 // Not working yet!
struct light_mod : public image_mod {
	Texture lightmap;
	IPF sub_ipf;
	light_mod(const vector<string>& args) : image_mod("L"), sub_ipf(join(args, ",")) {
		if(!sub_ipf.good)
			throw string("Invalid argument to L - must be a valid IPF chain");
		lightmap.set_image(sub_ipf.base_img);
		params.push_back(make_argument("lightmap", lightmap));
		for(auto& sub_mod : sub_ipf.mod_queue)
			copy(sub_mod->params.begin(), sub_mod->params.end(), back_inserter(params));
			/*
		ShaderFunction lm_fcn;
		lm_fcn.name = "calculate_lightmap";
		lm_fcn.result = "vec4";
		lm_fcn.params.push_back("texture2D lm");
		lm_fcn.params.push_back("vec3 tc");
		*/
	}
	void generate_code(vector<string>& code, vector<string>& files, const string& color_param) {
		code.push_back(GL_SETLINE(files.size()) + replace_all("$COLOR| = light($COLOR|, $LIGHT|);\n", {
			{"$COLOR|", color_param},
			{"$LIGHT|", "light_color"},
		}));
		files.push_back(__FILE__ "~L");
	}
	void generate_init_code(vector<string>& code, vector<string>& files, const string& tc_param) {
		code.push_back(GL_SETLINE(files.size()) + replace_all("\n"
"vec3 $\n"
"$CODE|\n"
"}\n",
		{
			{"$RESULT|", fcn->result},
			{"$NAME|", fcn->name},
			{"$PARAMS|", join(fcn->params, ",")},
			{"$CODE|", join(fcn->code, "\n")},
		}));
		files.push_back(__FILE__ "~L");
	}
};
#endif

shared_ptr<image_mod> image_mod::create(const string& code) {
	// Code must match the regex [A-Z]+\(.*?\)
	// (ignoring whitespace)
	string trimmed = trim(code);
	size_t args_start = trimmed.find_first_of('(');
	size_t args_end = trimmed.find_last_of(')');
	if(args_start == 0 || args_start == string::npos || args_end != trimmed.size() - 1) {
		cerr << "Invalid image path function: " << trimmed << '\n';
		return nullptr;
	}
	string name = trim(trimmed.substr(0, args_start));
	vector<string> args = split(trimmed.substr(args_start + 1, args_end - args_start - 1), ",");
	// Here we do the stuff that Wesnoth's split does implicitly...
	transform(args.begin(), args.end(), args.begin(), trim);
	args.erase(remove_if(args.begin(), args.end(), bind(&vector<string>::empty, &args)), args.end());
	try {
		if(name == "BLEND") return make_shared<blend_mod>(args);
		else if(name == "GS") return make_shared<gs_mod>(args);
		else if(name == "BW") return make_shared<bw_mod>(args);
		else if(name == "PAL") return make_shared<pal_mod>(args);
		else if(name == "RC") return make_shared<rc_mod>(args);
		else if(name == "CS") return make_shared<cs_mod>(args);
		else if(name == "R") return make_shared<r_mod>(args);
		else if(name == "G") return make_shared<g_mod>(args);
		else if(name == "B") return make_shared<b_mod>(args);
		else if(name == "NEG") return make_shared<neg_mod>(args);
		else if(name == "SWAP") return make_shared<swap_mod>(args);
		else if(name == "PLOT_ALPHA") return make_shared<plot_alpha_mod>(args);
		else if(name == "SEPIA") return make_shared<sepia_mod>(args);
		else if(name == "O") return make_shared<o_mod>(args);
		else if(name == "WIPE_ALPHA") return make_shared<wipe_alpha_mod>(args);
		#if 0 // WIP
		else if(name == "L") return make_shared<light_mod>(args);
		#endif
		else if(name == "BG") return make_shared<bg_mod>(args);
		else if(name == "FL") return make_shared<fl_mod>(args);
		else if(name == "ROTATE") return make_shared<rotate_mod>(args);
		else if(name == "SCALE") return make_shared<scale_mod>(args);
		else if(name == "SCALE_INTO") return make_shared<scale_into_mod>(args);
		else if(name != "NOP")
			cerr << "Unknown image path function: " << name << '\n';
	} catch(string& x) {
		cerr << x << '\n';
	}
	return nullptr;
}
