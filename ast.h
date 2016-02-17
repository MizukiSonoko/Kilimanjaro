#include <map>
#include <string>
#include <memory>

#include <iostream>
#include <sstream>

#include "token.h"

template<typename T>
bool TypeOf(std::string str,T type) {
    std::istringstream is(str);
    T v;
    is >> std::noskipws >> v;
    return is.eof() && !is.fail(); 
}

class AST{
	std::string name_;
	std::map<std::string,std::shared_ptr<AST>> childs;
    std::string valName;
    public:

    enum class Type{
        None = -1,
        Int,
        Float,
    } type;


	AST(std::string name, AST::Type type):
		name_(name),
        type(std::move(type))
	{}

    AST(std::string name):
        name_(name),
        type(AST::Type::None)
    {}

    AST(std::string name, std::string valName):
        name_(name),
        valName(valName),
        type(AST::Type::None)
    {}


    AST():
        name_(""),
        type(AST::Type::None)
    {}

	void append(std::string name,std::shared_ptr<AST> ast){
		childs[name] = std::move(ast);
	}

    std::string name() const{
        return name_;
    }

    std::string valueName() const{
        return valName;
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

    bool isInt(){
        return TypeOf(name_, 0);
    }

    bool isFloat(){
        return TypeOf(name_, 0.0f);
    }


    std::shared_ptr<AST> get(std::string name){
        if(childs.find(move(name)) != childs.end()){
            return move(childs[move(name)]);
        }else{
            throw name_+" must have "+name;
        }
    }
};