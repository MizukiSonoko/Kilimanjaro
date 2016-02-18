#include "token.h"

#include <map>

namespace AST{
    enum AstID{
        NONE = -1,
        FINID,
        Code,
        Function,
        Value,
        Number,
        Expression,
        Operator,
        BinaryASExpr,
        Identifire,
        VariableDeclID,
        StatementID
    };


    class AST{

            std::multimap<AstID, AST*> subAST;
            AstID type;
            std::string value;
        public:
            /* Using only speculate.*/
            bool isCorrect;
            AST(bool isC):
                type(NONE),value(""),isCorrect(isC){};

            AST():type(FINID),value("<FIN>"){};
            AST(AstID type):type(type){};
            AST(AstID type,std::string value):type(type),value(value){};
            ~AST(){
                for(auto it = subAST.begin(); it != subAST.end(); it++){ 
                    delete it->second;
                } 
            }
            auto add(AstID type, AST* obj)
             -> AST*{
                this->subAST.insert(std::make_pair( type, obj));
                return this;
            }
            auto print(int pos) 
             -> void{
                if( subAST.size() == 0 ){
                    //log(pos, AstID2s(type) + ":" + value );
                }else{
                    //log(pos, AstID2s(type) + ":" + value );
                    for(auto it = subAST.begin(); it != subAST.end(); ++it){ 
                        it->second->print(pos + 2);
                    } 
                }
            }

            std::multimap<AstID, AST*> getSubAST(){
                return subAST;
            }

            bool is(AstID id){
                return id == type;
            }

            bool has(AstID id){
                return subAST.find(id) != subAST.end();
            }

            std::string getValue(){
                return value;
            }

            std::vector<AST*> get(AstID id){
                std::vector<AST*> res;
                auto it  = subAST.lower_bound(id);
                auto end = subAST.upper_bound(id);
                while(it != end){
                    res.push_back( it-> second);
                    ++it;
                }                       
                return res;
            } 

            // For debug.
            std::string AstID2s(AstID id){
                switch(id){
                case NONE: return "NONE";
                case FINID: return "Fin";
                /*
                case NameID: return "Name";
                case NumberID: return "Number";
                case ListID: return "List";
                case OperatorID: return "Operator";
                case BinaryExprID: return "BinaryExpr";
                case IdentifireID: return "Identifire";
                case ListVariableDeclID: return "ListVariableDecl";
                case FunctionDeclID: return "functionTable";
                case ConditionExprID: return "ConditionExpr";
                case IfStatementID: return "IfStatement";
                case StatementID: return "Statement";
                case RightValueID: return "RightValue";
                case VariableDeclID: return "VariableDecl";
                */
                }
            }
    };
}
