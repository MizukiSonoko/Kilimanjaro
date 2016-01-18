#include "parser.h"

namespace parser{

    using namespace std;

    bool isFirst = true;

    int buf_index = 0;
    stack<int>       markers;
    vector<Token> headTokens;
    vector<Token> overHeadTokens;
    string         curString;

    list<Token> tokens;

    void log(int layour, string msg){
    #ifdef DEBUG
        for(int i=0; i < layour; i++){
            std::cout<< " ";
        }
        cout<< msg <<"\n";
    #endif
    }

    namespace Rule{
        map<string, vector< vector<pair<string,Token::Type>>>> rules;
        // Rules
        //std::map<AST::AstID, std::vector< std::function<AST::AST*(bool)>>> rules;

        auto setup()
         -> bool{
/*
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
*/            
            return true;
        }        
    }

    namespace Core{

        auto fill(int n)
         -> bool{
            log(13,"fill:"+std::to_string(n) + " "+std::to_string(tokens.size()));
            for(int i = 0;i < n;i++){
                headTokens.push_back(tokens.front());
                tokens.pop_front();
            }
            return true;
        }

        auto sync(int i)
         -> bool{
            log(13,"sync:"+std::to_string(i));
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

    bool match(Token::Type type){
            log(11, "match type");

            Token  token =  Core::LT(1);
            curString = token.value();
            Token::Type t = token.type();

            if(type == t){
                return Core::nextToken();
            }else if(curString == token.value()){
                return Core::nextToken();
            }else{
                return false;
            }
    }
/*
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
*/
    bool match(string name);
    
    bool match(vector<pair<string,Token::Type>> rules){
        log(6, "match");
        bool res = true;
        for(auto p : rules){
            log(7, "match [" + p.first + ","+ std::to_string(p.second) +"]");
            if(p.second == Token::NONE){
                res = res & match(p.first);
            }else{
                res = res & match(p.second);
            }
        }
        return res;
    }

    bool spec(vector< vector<pair<string,Token::Type>>> patterns){
        log(5, "spec");
        int count = 0;
        Core::mark();
        for(auto p : patterns){
            if(match(p)){
                Core::release();
                return count;
            }
            count++;
        }
        Core::release();
        return -1;
    }

    bool match(string name){
        log(3, "match ["+name+"]");
        auto patterns = Rule::rules[name];

        log(4, "match size of rule ["+std::to_string(patterns.size())+"]");
        int res = spec(patterns);
        log(4, "result:"+std::to_string(res));
        if(res == -1){
            return false;
        }
        return match(patterns[res]);
    }

    void loadRule(list<Token> rt){
        cout<<"Load rule set\n";
        map<string, vector< vector<pair<string,Token::Type>>>> ruleset;
        vector<pair<string,Token::Type>> tmps;

        int status = 0;
        string name = "";
        for(auto t : rt){
            switch(t.type()){
                case Token::LABRACKET:
                    if(status == 0){
                        status = 1;
                    }
                    break;
                case Token::IDENTIFIER:
                    if(status == 1){
                        name = t.value();
                        status = 2;
                    }else if(status == 6){
                        tmps.push_back(make_pair(t.value(), Token::NONE));                        
                    }
                    break;
                case Token::RABRACKET:
                    if(status == 2){
                        status = 3;
                    }
                    break;
                case Token::COLON:
                    if(status == 3 ){
                        status++;
                    }else if(status == 4){
                        status = 5;
                    }
                    break;
                case Token::EQUAL:
                    if(status == 5){
                        status = 6;
                    }
                    break;
                case Token::PERIOD:
                    if(status == 6){
                        status = 0;
                        cout<<"---- Rule ----\n";
                        cout << name << " -> ";
                        for(auto p : tmps){
                            cout << p.first <<" ";
                        }
                        cout<<"\n--------------\n";
                        Rule::rules[name].push_back(tmps);
                        tmps.clear();
                    }
                    break;
                default:
                    if(status == 6){
                        tmps.push_back(make_pair(t.value(), t.type()));
                    }
            }
        }
        Rule::rules["FIN"].push_back({make_pair("FIN", Token::FIN)});
        cout<<"Done\n";
    }

    AST::AST* parser(list<Token> t){
        log(0, "Parse start!");
        tokens = t;
        buf_index = 0;
        while(markers.size()!=0){
            markers.pop();
        }
        headTokens.clear();
        auto res = match("PS");
        cout << res << endl;
        return nullptr;
    }
}