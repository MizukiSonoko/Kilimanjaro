#pragma once

#include <string>

class Token{
  public:
	enum Type{
			NONE = -2,
	        ERROR = -1,
	        MARGIN,
	        FIN,
	        NAME,
	        NUMBER,
	        REALNUMBER,
	        IDENTIFIER,
	        EXCLAMATION,
	        LPARENT,
	        RPARENT,
	        RBRACKET,
	        LBRACKET,   // 10
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
		type_(std::move(aType)),
		value_(std::move(aValue))
	{}

	Type type() const{
		return type_;
	}
	std::string value() const{
		return value_;
	}
  private:
	Type type_;
	std::string value_;

};
