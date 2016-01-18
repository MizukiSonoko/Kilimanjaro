#pragma once

#include <iostream>
#include <cassert>
#include <list>
#include <vector>
#include <fstream>

#include "token.h"

namespace lexser{
	using namespace std;
	
	namespace decide{
		bool isNumber(char c);
		bool isLetter(char c);
		bool isAlphanumeric(char c);
	};

	bool loader(string filename);
	void lexser();


};