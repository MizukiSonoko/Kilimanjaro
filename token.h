#pragma once

#include <string>

class Token{
  public:
	enum Type{
			NONE = -2,
	        ERROR = -1,
	        MARGIN,       // 0
	        FIN,          
	        NAME,         // 2
	        NUMBER,       // 3
	        REALNUMBER,   // 4
	        IDENTIFIER,   // 5
	        EXCLAMATION,
	        LPARENT,
	        RPARENT,
	        RBRACKET,
	        LBRACKET,
	        RABRACKET,	        
	        LABRACKET,
	        RCBRACKET,
	        LCBRACKET,
	        SEMICOLON,
	        SHARPE,
	        WAVY,
	        COLON,
	        COMMA,
	        PERIOD,
	        CARET,
	        AT_SIGN,
	        DQUOTATION,
	        EQUAL,
	        OPE_ADD, 
	        OPE_SUB, 
	        OPE_MUL, 
	        OPE_DIV, 
	};

	Token(Type aType,std::string aValue):
		type_(aType),
		value_(aValue){}

	Type type(){
		return type_;
	}
	std::string value(){
		return value_;
	}
  private:
	Type type_;
	std::string value_;

};
