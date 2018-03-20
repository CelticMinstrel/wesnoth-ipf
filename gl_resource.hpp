
#pragma once

struct gl_resource {
	gl_resource(unsigned int id, void(*deleter)(unsigned int))
		: id(id), del(deleter)
	{}
	~gl_resource() {del(id);}
	unsigned int id;
	void(*del)(unsigned int);
};
