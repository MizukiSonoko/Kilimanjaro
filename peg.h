
#include "token.h"

#include <cstdlib>

#include <initializer_list>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <list>
#include <vector>

namespace peg{

	void load_cpp(std::string filename);
	void load_rule(std::string filename);
	bool exec(std::string n);
	void test();
}