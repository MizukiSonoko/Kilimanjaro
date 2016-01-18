#include "token.h"
#include "ast.h"

#include <functional>
#include <list>

namespace parser{

    using namespace std;

    AST::AST* parser(list<Token> tokens);
}