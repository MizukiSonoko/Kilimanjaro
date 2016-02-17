#pragma once

#include "ast.h"

#include <functional>
#include <initializer_list>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stack>
#include <map>
#include <iostream>
#include <list>

namespace parser{

    using namespace std;

    void loadRule(list<Token> tokens);
    AST<void>* parser(list<Token> tokens);
}