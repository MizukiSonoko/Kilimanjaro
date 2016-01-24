
#include "token.h"

#include <cstdlib>

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <list>
#include <vector>

namespace peg{

	void load_cpp(std::string filename);
	void load_rule(std::string filename);
	bool exec();
}