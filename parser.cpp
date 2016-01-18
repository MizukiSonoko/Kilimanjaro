#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stack>
#include <map>

#include <iostream>

#include "parser.h"


namespace parser{

    using namespace std;

    bool isFirst = true;

    int buf_index = 0;
    std::stack<int>       markers;
    std::vector<Token> headTokens;
    std::vector<Token> overHeadTokens;
    std::string         curString;

    list<Token> tokens;

    void log(int layour, std::string msg){
    #ifdef DEBUG
        for(int i=0; i < layour; i++){
            std::cout<< " ";
        }
        std::cout<< msg <<"\n";
    #endif
    }

    namespace Core{

        auto fill(int n)
         -> bool{

            for(int i = 0;i < n;i++){
                headTokens.push_back(tokens.front());
                tokens.pop_front();
            }
            return true;
        }

        auto sync(int i)
         -> bool{
            if(buf_index + i > headTokens.size()){
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
            if(buf_index == headTokens.size() && !isSpec()){
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
        if(type == t and curString == reserved){
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

        auto setup()
         -> bool{

            {//Fin
                rules[AST::FINID].push_back( 
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
            {//Number
                rules[AST::NumberID].push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){
                            if(
                                match(Token::NUMBER) &&
                                match(Token::PERIOD) &&
                                match(Token::NUMBER)){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);
                            }
                        }else{
                            match(Token::NUMBER);
                            std::string _int = curString;
                            match(Token::PERIOD);
                            match(Token::NUMBER);
                            std::string _frac = curString;
                            return new AST::AST(AST::NumberID, _int + "." + _frac);
                        }
                    }
                );
                rules[AST::NumberID].push_back(
                    [](bool isSpec) -> AST::AST*{
                        if(isSpec){                       
                            if( match(Token::NUMBER) ){
                                return new AST::AST(true);
                            }else{
                                return new AST::AST(false);                                
                            }
                        }else{
                            match(Token::NUMBER);
                            return new AST::AST(AST::NumberID, curString);
                        }
                    }
                );
            }
            return true;
        }        
    }

    AST::AST* parser(list<Token> t){
        tokens = t;
        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();   
        
        if(isFirst){
            Rule::setup();     
            isFirst = false;
        }
        return nullptr;
    }
}