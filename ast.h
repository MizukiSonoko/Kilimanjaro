#include <map>
#include <string>

template<typename T>
class AST{
	string name;
	Type type;
	map<string,AST> childs;

	public:
	AST(string name,Type type):
		name(move(name)),
		type(move(type))
	{}

	append(string name,AST ast){
		childs[name] = move(ast);
	}

};