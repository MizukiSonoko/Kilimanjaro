#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <stack>
#include <map>
#include <functional>
#include <initializer_list>
#include <array> 
#include <fstream>
 
#include <cstdarg>
#include <memory>
#include <iterator>

#include <regex>
#include <chrono>
#include <iomanip>

#include "parser.h"

#define RELEASE(x) {delete x;x=NULL;}
#define RELEASEA(x) {delete[] x;x=NULL;}

std::map<std::string, int> values;

namespace parser{

    class ValueException : public exception{
        string msg;
      public:
        ValueException(string m):
          msg(move(m))
        {}

        const char* what () const throw (){
          return ("\n[Exception] "+msg).c_str();
        }
    };

    std::list<Token>  tokens;

    bool isFirst = true;

    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;
    std::vector<Token> overHeadTokens;
    std::string         curString;

    std::map<std::string, int> variableTable;
    std::map<std::string, int> functionTable;

    std::string cur_val_name;

    void log(int layour, std::string msg){
    #ifdef DEBUG
        for(int i=0; i < layour; i++){
            std::cout<< " ";
        }
        std::cout<< msg <<"\n";
    #endif
    }


    auto defVariable(std::string val_name)
         -> bool{
        if(variableTable.find(val_name) == variableTable.end()){
            return false;
        }
        return true;
    }

    namespace Core{

        auto fill(int n)
         -> bool{
            if(n > tokens.size())
                n = tokens.size();
                
            for(int i = 0;i < n;i++){
                headTokens.push_back(move(tokens.front()));
                tokens.pop_front();
            }
            return true;
        }

        auto sync(int i)
         -> bool{
            if(((unsigned)buf_index + i) > headTokens.size()){
                int n = (buf_index + i) - (headTokens.size());
                fill(n);
            }
            return true;
        }

        auto LT(int i)
         -> Token{        
            sync(i);
            return headTokens[buf_index+i-1];
        }
        
        auto margin(int i)
         -> Token{
            return headTokens[buf_index+i-2];
        }

        auto mark()
         -> int{
            markers.push(buf_index);
            return buf_index;
        }

        auto seek(int index)
         -> bool{
            buf_index = index;
            return true;
        }

        auto release()
         -> bool{
            if(!markers.empty()){
                int marker = markers.top();
                markers.pop();
                seek(marker);
            }
            return true;
        }

        auto isSpec()
         -> bool{
            return markers.size() > 0;
        }

        auto nextToken()
         -> bool{
            buf_index++;
            if(((unsigned)buf_index) == headTokens.size() && !isSpec()){
                buf_index = 0;
                headTokens.clear();
                return false; 
            }
            sync(1);   
            return true;
        }       

    }

    namespace speculate{
        auto speculate(std::function<AST::AST*(bool)> rule)
         -> bool{

            Core::mark();
            bool success = false;
            if(
                rule(true)->isCorrect
            ){
                success = true;
            }
            Core::release();
            return success;
        }

        auto speculate(std::vector< std::function<AST::AST*(bool)>> rules)
         -> int{
            int case_num = 1;
            for(auto rule : rules){
                if(speculate(rule)){
                    return case_num;
                }
                case_num ++;
            }
            return 0;
        }
    };

    auto match(Token::Type type)
         -> bool{
            Token  token =  Core::LT(1);     
            curString = token.value();

            Token::Type t = token.type();
            if(type == t){
                Core::nextToken();
                return true;
            }else{
                return false;
            }
    }

    auto match(Token::Type type, std::string reserved)
     -> bool{
        Token  token =  Core::LT(1);     
     
        curString = token.value();

        Token::Type t = token.type();
        if(type == t and curString == move(reserved)){
            Core::nextToken();
            return true;
        }else{
            return false;           
        }
    } 

    auto match(std::vector< std::function<AST::AST*(bool)>> rules)
     -> AST::AST* {        
        int _result = speculate::speculate(rules);
        if(!_result){
            return nullptr;
        }
        return rules[ _result-1 ](false);
    }
    
    namespace Rule{
        // Rules
        std::map<AST::AstID, std::vector< std::function<AST::AST*(bool)>>> rules;
        std::vector< std::function<AST::AST*(bool)>> Number;
        std::vector< std::function<AST::AST*(bool)>> List;
        std::vector< std::function<AST::AST*(bool)>> Operator;
        std::vector< std::function<AST::AST*(bool)>> ListVariableDecl;
        std::vector< std::function<AST::AST*(bool)>> ConditionExpr;
        std::vector< std::function<AST::AST*(bool)>> IfStatement;
        std::vector< std::function<AST::AST*(bool)>> RightValue;

        std::vector< std::function<AST::AST*(bool)>> FIN;
        std::vector< std::function<AST::AST*(bool)>> Identifire;
        std::vector< std::function<AST::AST*(bool)>> BinaryASExpr;
        std::vector< std::function<AST::AST*(bool)>> Expression;
        std::vector< std::function<AST::AST*(bool)>> VariableDecl;
        std::vector< std::function<AST::AST*(bool)>> Statement;
        std::vector< std::function<AST::AST*(bool)>> Function;        
        std::vector< std::function<AST::AST*(bool)>> Code;

        std::vector< std::function<AST::AST*(bool)>> TestCore;

        auto setup()
         -> bool{

            {//Fin
                FIN.push_back( 
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::FIN) ){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            return new AST::AST(AST::FINID,"<FIN>");
                        }
                    }
                );
            }

            {// Identifire
                Identifire.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::IDENTIFIER) ){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::IDENTIFIER);
                            return (new AST::AST(AST::Identifire, curString));      
                        }
                    }
                ); 
                Identifire.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::NUMBER)){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::NUMBER);
                            return (new AST::AST(AST::Number, curString));      
                        }
                    }
                );   
            }

            {//BinaryASExpr
                BinaryASExpr.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Identifire) &&
                                match(Token::OPE_ADD) &&
                                match(Identifire)){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{

                            cout<< "-> id + id ";
                            auto id1 = match(Identifire);
                            match(Token::OPE_ADD);
                            auto id2 = match(Identifire);                            

                            return (new AST::AST(AST::BinaryASExpr))
                                ->add(AST::Identifire, id1)
                                ->add(AST::Operator,new  AST::AST(AST::Operator, "+"))
                                ->add(AST::Identifire, id2);                            
                        }
                    }
                );  
            }
            {// Expression
                Expression.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Identifire)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            cout<< "-> identifire ";
                            auto identifire = match(Identifire);
                            return identifire;
                        }
                    }
                );

                Expression.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(BinaryASExpr)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            cout<< "-> binASExpr( ";
                            auto binExpr = match(BinaryASExpr);
                            return binExpr;
                        }
                    }
                );
            }

            {//VariableDecl
                VariableDecl.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::IDENTIFIER, "val") &
                                match(Token::IDENTIFIER) &&
                                match(Token::EQUAL) &&
                                match(Expression) &&
                                match(Token::SEMICOLON)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            cout<< "-> val id = Expression( ";
                            match(Token::IDENTIFIER, "val");
                            match(Token::IDENTIFIER);
                            auto name = curString;
                            log(0,"define "+ curString);

                            match(Token::EQUAL);                            
                            auto expression = match(Expression);
                            match(Token::SEMICOLON);
                            return (new AST::AST(AST::VariableDeclID))
                                ->add(AST::Value, new AST::AST(AST::Value, name))
                                ->add(AST::Expression, expression);
                        }
                    }
                );
            }

            {// Statement
                Statement.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(VariableDecl) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            cout<< "-> VariableDecl( ";
                            auto variableDecl = match(VariableDecl);
                            return (new AST::AST(AST::StatementID))
                                ->add(AST::VariableDeclID, variableDecl);
                        }
                    }
                );
            }

            {// Function
                Function.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::IDENTIFIER, "def") &&
                                match(Token::IDENTIFIER) &&
                                match(Token::LPARENT) &&
                                match(Token::RPARENT) && 
                                match(Token::LCBRACKET) &&
                                match(Statement) &&
                                match(Token::RCBRACKET)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            cout<< "-> def id() { Statement } ";
                            match(Token::IDENTIFIER, "def");
                            match(Token::IDENTIFIER);

                            auto name = curString;

                            match(Token::LPARENT);
                            match(Token::RPARENT);
                            match(Token::LCBRACKET);

                            auto statement = match(Statement);

                            match(Token::RCBRACKET); 
                            return (new AST::AST(AST::Function, name));
                        }
                    }
                );

                Function.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Token::IDENTIFIER, "def") &&
                                match(Token::IDENTIFIER) &&
                                match(Token::LPARENT) &&
                                match(Token::RPARENT) && 
                                match(Token::LCBRACKET) &&
                                match(Token::RCBRACKET)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            cout<< "-> def id() { } ";
                            match(Token::IDENTIFIER, "def");
                            match(Token::IDENTIFIER);

                            auto name = curString;

                            match(Token::LPARENT);
                            match(Token::RPARENT);
                            match(Token::LCBRACKET);

                            match(Token::RCBRACKET); 
                            return (new AST::AST(AST::Function, name));
                        }
                    }
                );
            }

            {//Code
                Code.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            return match(FIN) ?
                                new AST::AST(true) :
                                new AST::AST(false);
                        }else{
                            match(FIN);
                            return (new AST::AST(AST::Code));
                        }
                    }
                );

                Code.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Function) &&
                                match(FIN) ){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            auto function = match(Function);
                            match(FIN);
                            return (new AST::AST(AST::Code))
                                ->add(AST::Function, function);
                        }
                    }
                );

                Code.push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if( match(Function) &&
                                match(Code) &&
                                match(FIN) ){
                                return new AST::AST(true);   
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            auto function = match(Function);
                            auto code = match(Code);
                            match(FIN);
                            return (new AST::AST(AST::Code))
                                ->add(AST::Function, function)
                                ->add(AST::Code, code);
                        }
                    }
                );
            }

            return true;
        }        
    }

    auto parser(list<Token> t)
     -> AST::AST*{

        tokens = move(t);

        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();   
        
        if(isFirst){
            Rule::setup();     
            isFirst = false;
        }
        
        auto result = match(Rule::Code);
        cout << result << endl;
        return result;
    }
}
