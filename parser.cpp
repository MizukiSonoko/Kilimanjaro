#include "parser.h"

namespace parser{


    AST::AST* parser(list<Token> t){
        log(log_pos, "Parse start!");
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