
#include "llvm.h"

namespace CodeGen{

    using namespace std;

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


    struct CodeGenContext{
        llvm::LLVMContext& context;
        llvm::Module* module;
        llvm::IRBuilder<> builder;
        map<string, llvm::Value*> NameTable;
        CodeGenContext():
            context(llvm::getGlobalContext()),
            module(new llvm::Module(move("default"), context )),
            builder(context)
        {}

        CodeGenContext(string moduleName):
            context(llvm::getGlobalContext()),
            module(new llvm::Module( move(moduleName), context )),
            builder(context)
        {}

    }; 

    shared_ptr<CodeGenContext> context;

    llvm::BinaryOperator* addExpr(llvm::Value* lhs,llvm::Value* rhs){
      return llvm::BinaryOperator::CreateAdd(lhs, rhs);
    }

    void init(){
        context = make_shared<CodeGenContext>("sharo");
    }

    llvm::Value* ast2value(shared_ptr<AST::AST> ast);
    
    template<typename T>
    llvm::Value* makeValue(T v){
      throw ValueException("Not implement makeValue");
    }

    template<>
    llvm::Value* makeValue<int>(int v){
      bool  is_signed = false;
      return llvm::ConstantInt::get(context->context, llvm::APInt(/*nbits*/32, v, /*bool*/is_signed));
    }

    template<>
    llvm::Value* makeValue<float>(float v){
      return llvm::ConstantFP::get(context->context, llvm::APFloat(v));
    }

    template<>
    llvm::Value* makeValue<double>(double v){
      return llvm::ConstantFP::get(context->context, llvm::APFloat(v));
    }

    llvm::Value* makeBinMulDivExpr(shared_ptr<AST::AST> ast){
        auto right = ast->get(AST::AstID::Right);
        auto left  = ast->get(AST::AstID::Left);
        auto ope   = ast->get(AST::AstID::Operator);
        if(ope->is(AST::AstID::OPE_MUL)){
            return context->builder.CreateMul(
               ast2value(move(right)),
               ast2value(move(left)),
              "multmp"
            );
        }else if(ope->is(AST::AstID::OPE_DIV)){
            return context->builder.CreateUDiv(
               ast2value(move(right)),
               ast2value(move(left)),
              "divtmp"
            );
        }
        throw ValueException("Not implement makeBinMulDivExpr");
    }
    
    llvm::Value* makeBinAddSubExpr(shared_ptr<AST::AST> ast){
        auto right = ast->get(AST::AstID::Right);
        auto left  = ast->get(AST::AstID::Left);
        auto ope   = ast->get(AST::AstID::Operator);
        if(ope->is(AST::AstID::OPE_ADD)){
            return context->builder.CreateAdd(
               ast2value(move(right)),
               ast2value(move(left)),
              "addtmp"
            );
        }else if(ope->is(AST::AstID::OPE_SUB)){
            return context->builder.CreateSub(
               ast2value(move(right)),
               ast2value(move(left)),
              "subtmp"
            );
        }
        throw ValueException("Not implement makeBinAddSubExpr");
    }

    llvm::Value* VariableDecl(shared_ptr<AST::AST> ast){
        auto value = ast->get(AST::AstID::Value);
        auto expr  = ast->get(AST::AstID::Expression);
        /*/ 上書きは今回認められないわぁ
        if(context->NameTable.find(value->name()) != context->NameTable.end()){      
            throw ValueException(value->name() + " is already defined!");
        }
        // */ 
        auto val = ast2value(move(expr));
        context->NameTable[value->value()] = val;
        return val;
    }

    llvm::Value* VariableDecl(string name, llvm::Value* val){
        /*/ 上書きは今回認められないわぁ
        if(context->NameTable.find(name) != context->NameTable.end()){      
            throw ValueException(name + " is already defined!");
        }
        */
        context->NameTable[name] = val;
        return val;
    }

    llvm::Value* VariableOverride(string name, llvm::Value* val){
        // 上書きは認めるわぁ
        context->NameTable[name] = val;
        return val;
    }

    void VariableRemove(string name){
      context->NameTable.erase(context->NameTable.find(name));
    }


    llvm::Value* getValue(shared_ptr<AST::AST> ast){
        auto name = ast->name();
        if(context->NameTable.find(name) != context->NameTable.end())
            return context->NameTable[name];
        throw ValueException(name+" is undefined value");
    }

    llvm::Value* getValue(string name){
        if(context->NameTable.find(name) != context->NameTable.end())
            return context->NameTable[name];
        throw ValueException(name+" is undefined value");
    }

    llvm::Value* ifStatement(shared_ptr<AST::AST> ast){
        llvm::Value* condVal = ast2value(ast->get(AST::AstID::Condition));
        if (!condVal){
          return nullptr;
        }

        condVal = context->builder.CreateICmpNE( condVal, makeValue(0), "ifcond");

        llvm::Function *function = context->builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *thenBlock  = llvm::BasicBlock::Create( context->context, "then", function);
        llvm::BasicBlock* elseBlock  = llvm::BasicBlock::Create( context->context, "else");
        llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create( context->context, "ifcont");

        context->builder.CreateCondBr(condVal, thenBlock, elseBlock);

        context->builder.SetInsertPoint(thenBlock);

        llvm::Value* thenVal= ast2value(ast->get(AST::AstID::Then));
        if (!thenVal)
          return nullptr;

        context->builder.CreateBr(mergeBlock);
        thenBlock = context->builder.GetInsertBlock();

        function->getBasicBlockList().push_back(elseBlock);
        context->builder.SetInsertPoint(elseBlock);

        llvm::Value* elseVal = ast2value(ast->get(AST::AstID::Else));
        if (!elseVal)
          return nullptr;

        context->builder.CreateBr(mergeBlock);
        elseBlock = context->builder.GetInsertBlock();

        function->getBasicBlockList().push_back(mergeBlock);
        context->builder.SetInsertPoint(mergeBlock);
        llvm::PHINode *PN =
            context->builder.CreatePHI(llvm::Type::getInt32Ty(context->context), 2, "iftmp");

        PN->addIncoming(thenVal, thenBlock);
        PN->addIncoming(elseVal, elseBlock);
        return PN;
    }

// Output for-loop as:
//   ...
//   start = startexpr
//   goto loop
// loop:
//   variable = phi [start, loopheader], [nextvariable, loopend]
//   ...
//   bodyexpr
//   ...
// loopend:
//   step = stepexpr
//   nextvariable = variable + step
//   endcond = endexpr
//   br endcond, loop, endloop
// outloop:

    llvm::Value* forStatement(shared_ptr<AST::AST> ast){
        llvm::Value* startVal = ast2value(ast->get(AST::AstID::START));
        if (!startVal)
            return nullptr;

        string valueName = ast->get(AST::AstID::Value)->name();
        llvm::Function *function = context->builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *preheaderBlock = context->builder.GetInsertBlock();
        llvm::BasicBlock *loopBlock = llvm::BasicBlock::Create( context->context, "loop", function);

        context->builder.CreateBr(loopBlock);
        context->builder.SetInsertPoint(loopBlock);

        llvm::PHINode *variable = context->builder.CreatePHI(llvm::Type::getInt32Ty( context->context), 2, valueName.c_str());
        variable->addIncoming(startVal, preheaderBlock);

        // Within the loop, the variable is defined equal to the PHI node.  If it
        // shadows an existing variable, we have to restore it, so save it now.
        llvm::Value *oldVal = getValue(valueName);
        VariableDecl( valueName, variable);

        // Emit the body of the loop.  This, like any other expr, can change the
        // current BB.  Note that we ignore the value computed by the body, but don't
        // allow an error.
        ast2value(ast->get(AST::AstID::Body));

        // Emit the step value.
        llvm::Value* stepVal = nullptr;
        stepVal = llvm::ConstantFP::get( context->context, llvm::APFloat(1.0));

        llvm::Value* nextVal = context->builder.CreateFAdd(variable, stepVal, "nextvar");

        // Compute the end condition.
        llvm::Value* endCond = ast2value( ast->get(AST::AstID::Condition));
        if (!endCond)
          return nullptr;

        endCond = context->builder.CreateICmpNE( endCond, makeValue(1), "loopcond");

        llvm::BasicBlock *loopEndBlock = context->builder.GetInsertBlock();
        llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create( context->context, "afterloop", function);

        // Insert the conditional branch into the end of LoopEndBB.
        context->builder.CreateCondBr(endCond, loopBlock, afterBlock);

        // Any new code will be inserted in AfterBB.
        context->builder.SetInsertPoint(afterBlock);

        // Add a new entry to the PHI node for the backedge.
        variable->addIncoming(nextVal, loopEndBlock);

        // Restore the unshadowed variable.
        if (oldVal){
          VariableOverride(valueName, oldVal);
        }else{
          VariableRemove(valueName);
        }

        // for expr always returns 0.0.
        return llvm::Constant::getNullValue(llvm::Type::getDoubleTy( context->context ));
    }


    llvm::Value* Return(shared_ptr<AST::AST> ast){
        auto value = ast->get(AST::AstID::Value);
        return context->builder.CreateRet(ast2value(value));
    }

    llvm::Value* ast2value(shared_ptr<AST::AST> ast){
/*      
        if(ast->is()){
            return makeValue(ast->asInt());
        }else if(ast->isFloat()){
            return makeValue(ast->asFloat());
        }else if(ast->is("BinAddSubExpr")){
            return makeBinAddSubExpr(move(ast));
        }else if(ast->is("BinMulDivExpr")){
            return makeBinMulDivExpr(move(ast));
*/
        if(ast->is(AST::AstID::Value)){
            return getValue(move(ast));
/*
        }else if(ast->is("VariableDecl")){
            return VariableDecl(move(ast));
        }else if(ast->is("Return")){
            return Return(ast);
        }else if(ast->is("ifStatement")){
            return ifStatement(ast);
        }else if(ast->is("forStatement")){
            return ifStatement(ast);
*/            
        }
        throw ValueException("Not implement ast2value");
    }

    void setEntry(llvm::Function* function){
      llvm::BasicBlock* entry = llvm::BasicBlock::Create( context->context, "entry", function);
      context->builder.SetInsertPoint(entry);
    }

    template<typename T>
    llvm::Function* makeFunction(string  name, vector<shared_ptr<AST::AST>> asts){
      return nullptr;
    }

    template<>
    llvm::Function* makeFunction<int>(string  name, vector<std::shared_ptr<AST::AST>> statements){
      auto function = llvm::Function::Create(
          llvm::FunctionType::get( context->builder.getInt32Ty(), false ),
          llvm::Function::ExternalLinkage,
          name,
          context->module
        );
      setEntry(function);

      for(auto& st : statements){
        ast2value(move(st));
      }

      return function;
    }

    template<>
    llvm::Function* makeFunction<void>(string  name, vector<shared_ptr<AST::AST>> asts){
      return  llvm::Function::Create(
          llvm::FunctionType::get( context->builder.getVoidTy(), false ),
          llvm::Function::ExternalLinkage,
          name,
          context->module
        );
    }

    void dump(){
      context->module->dump();
    }
};

/*
std::shared_ptr<AST::AST> gForStatement(std::shared_ptr<AST::AST> start, std::shared_ptr<AST::AST> value, std::shared_ptr<AST::AST> body, std::shared_ptr<AST::AST> cond){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("forStatement");
    result->append("start", move(start));
    result->append("value", move(value));
    result->append("body", move(body));
    result->append("cond", move(cond));
    return result;
}

std::shared_ptr<AST::AST> gIfStatement(std::shared_ptr<AST::AST> cond, std::shared_ptr<AST::AST> then, std::shared_ptr<AST::AST> els){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("ifStatement");
    result->append("cond", move(cond));
    result->append("then", move(then));
    result->append("else", move(els));
    return result;
}

std::shared_ptr<AST::AST> gReturn(std::shared_ptr<AST::AST> value){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("Return");
    result->append("Value", move(value));
    return result;
}


std::shared_ptr<AST::AST> gValDecl(std::string name, std::shared_ptr<AST::AST> expr){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("VariableDecl");
    result->append("Value", CodeGen::make_shared<AST::AST>(name));
    result->append("Expr", move(expr));
    return result;
}

std::shared_ptr<AST::AST> gAdd(std::shared_ptr<AST::AST> l,std::shared_ptr<AST::AST> r){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("BinAddSubExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST::AST>("+"));
    return result;
}

std::shared_ptr<AST::AST> gSub(std::shared_ptr<AST::AST> l,std::shared_ptr<AST::AST> r){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("BinAddSubExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST::AST>("-"));
    return result;
}

std::shared_ptr<AST::AST> gMul(std::shared_ptr<AST::AST> l,std::shared_ptr<AST::AST> r){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("BinMulDivExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST::AST>("*"));
    return result;
}

std::shared_ptr<AST::AST> gDiv(std::shared_ptr<AST::AST> l,std::shared_ptr<AST::AST> r){
    std::shared_ptr<AST::AST> result = CodeGen::make_shared<AST::AST>("BinMulDivExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST::AST>("/"));
    return result;
}

std::vector<CodeGen::shared_ptr<AST::AST>> generateTestAst(){
  std::vector<CodeGen::shared_ptr<AST::AST>> result;
  


  // val sharo = 1;
  result.push_back( gValDecl("sharo", CodeGen::make_shared<AST::AST>("3", AST::Type::Int)));

  // if sharo:
  //    chino = 1
  // else:
  //    chino = 3
  result.push_back( gIfStatement(CodeGen::make_shared<AST::AST>("Value","sharo"),
      gValDecl("chino", CodeGen::make_shared<AST::AST>("1", AST::Type::Int)),
      gValDecl("chino", CodeGen::make_shared<AST::AST>("3", AST::Type::Int))
  ));

  // val hoge = chino + 2;
  std::shared_ptr<AST::AST> one =  CodeGen::make_shared<AST::AST>("Value","chino"); 
  std::shared_ptr<AST::AST> two = CodeGen::make_shared<AST::AST>("2", AST::Type::Int);
  result.push_back( gValDecl("hoge", gAdd(move(one), move(two))));

  // val huga = hoge + 3;
  std::shared_ptr<AST::AST> three = CodeGen::make_shared<AST::AST>("3", AST::Type::Int);
  std::shared_ptr<AST::AST> hoge = CodeGen::make_shared<AST::AST>("Value","hoge");
  result.push_back( gValDecl("huga", gMul(move(three), hoge)));

  // return hoge + huga
  result.push_back( gReturn(gAdd(move(hoge), CodeGen::make_shared<AST::AST>("Value","huga"))));
  return result;
}
*/
