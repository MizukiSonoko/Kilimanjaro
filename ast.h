#include <map>
#include <string>

class Type{
    
};
template<typename T>
class AST{
	std::string name;
	Type type;
	std::map<std::string,AST> childs;

	public:
	AST(std::string name,Type type):
		name(std::move(name)),
		type(std::move(type))
	{}

	void append(std::string name,AST ast){
		childs[name] = std::move(ast);
	}
};