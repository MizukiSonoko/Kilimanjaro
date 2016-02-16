#include <map>
#include <string>
#include <memory>

class AST{
	std::string name_;
	std::map<std::string,std::unique_ptr<AST>> childs;

    public:

    enum class Type{
        None = -1,
        Int
    } type;

	AST(std::string name, AST::Type type):
		name_(std::move(name)),
        type(std::move(type))
	{}

    AST(std::string name):
        name_(std::move(name)),
        type(AST::Type::None)
    {}

    AST():
        name_(""),
        type(AST::Type::None)
    {}

	void append(std::string name,std::unique_ptr<AST> ast){
		childs[name] = std::move(ast);
	}

    std::string name() const{
        return name_;
    }

    bool is(std::string name){
        return name_ == name;
    }

    int asInt(){
        try{
            return std::stoi(name_);
        }catch(std::invalid_argument e) {
            throw name_+" is not number";
        }
    }

    float asFloat(){
        try{
            return std::stof(name_);
        }catch(std::invalid_argument e) {
            throw name_+" is not number";
        }
    }


    std::unique_ptr<AST> get(std::string name){
        if(childs.find(name) != childs.end()){
            return move(childs[name]);
        }else{
            throw name_+" must have "+name;
        }
    }
};