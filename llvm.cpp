#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/ValueHandle.h>

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <exception>

#include "ast.h"

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
        map<string, shared_ptr<llvm::Value>> NameTable;
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

    shared_ptr<llvm::BinaryOperator> addExpr(shared_ptr<llvm::Value> lhs,shared_ptr<llvm::Value> rhs){
      return shared_ptr<llvm::BinaryOperator>(llvm::BinaryOperator::CreateAdd(lhs.get(),rhs.get()));
    }

    void init(){
        context = make_shared<CodeGenContext>("sharo");
    }

    shared_ptr<llvm::Value> ast2value(shared_ptr<AST> ast);
    
    template<typename T>
    shared_ptr<llvm::Value> makeValue(T v){
      throw ValueException("Not implement makeValue");
    }

    template<>
    shared_ptr<llvm::Value> makeValue<int>(int v){
      bool  is_signed = false;
      return shared_ptr<llvm::Value>(llvm::ConstantInt::get(context->context, llvm::APInt(/*nbits*/32, v, /*bool*/is_signed)));
    }

    template<>
    shared_ptr<llvm::Value> makeValue<float>(float v){
      return shared_ptr<llvm::Value>(llvm::ConstantFP::get(context->context, llvm::APFloat(v)));
    }

    template<>
    shared_ptr<llvm::Value> makeValue<double>(double v){
      return shared_ptr<llvm::Value>(llvm::ConstantFP::get(context->context, llvm::APFloat(v)));
    }

    shared_ptr<llvm::Value> makeBinMulDivExpr(shared_ptr<AST> ast){
        auto right = ast->get("right");
        auto left  = ast->get("left");
        auto ope   = ast->get("operator");
        if(ope->is("*")){
            return shared_ptr<llvm::Value>(context->builder.CreateMul(
               ast2value(move(right)).get(),
               ast2value(move(left)).get(),
              "multmp"
            ));
        }else if(ope->is("/")){
            return shared_ptr<llvm::Value>(context->builder.CreateUDiv(
               ast2value(move(right)).get(),
               ast2value(move(left)).get(),
              "divtmp"
            ));
        }
        throw ValueException("Not implement makeBinMulDivExpr");
    }
    
    shared_ptr<llvm::Value> makeBinAddSubExpr(shared_ptr<AST> ast){
        auto right = ast->get("right");
        auto left  = ast->get("left");
        auto ope   = ast->get("operator");
        if(ope->is("+")){
            return shared_ptr<llvm::Value>(context->builder.CreateAdd(
               ast2value(move(right)).get(),
               ast2value(move(left)).get(),
              "addtmp"
            ));
        }else if(ope->is("-")){
            return shared_ptr<llvm::Value>(context->builder.CreateSub(
               ast2value(move(right)).get(),
               ast2value(move(left)).get(),
              "subtmp"
            ));
        }
        throw ValueException("Not implement makeBinAddSubExpr");
    }

    shared_ptr<llvm::Value> ast2value(shared_ptr<AST> ast){
        if(ast->isInt()){
            return makeValue(ast->asInt());
        }else if(ast->isFloat()){
            return makeValue(ast->asFloat());
        }else if(ast->is("BinAddSubExpr")){
            return makeBinAddSubExpr(move(ast));
        }else if(ast->is("BinMulDivExpr")){
            return makeBinMulDivExpr(move(ast));
        }else if(ast->is("Expression")){
            throw ValueException("Not implement ast2value");
        }
        throw ValueException("Not implement ast2value");
    }


    void setEntry(llvm::Function* function){
      llvm::BasicBlock* entry = llvm::BasicBlock::Create( context->context, "entry", function);
      context->builder.SetInsertPoint(entry);
    }

    template<typename T>
    llvm::Function* makeFunction(string  name, vector<shared_ptr<AST>> asts){
      return nullptr;
    }

    template<>
    llvm::Function* makeFunction<int>(string  name, vector<std::shared_ptr<AST>> statements){
      auto function = llvm::Function::Create(
          llvm::FunctionType::get( context->builder.getInt32Ty(), false ),
          llvm::Function::ExternalLinkage,
          name,
          context->module
        );
      setEntry(function);

      llvm::Value* result;
      for(auto& st : statements){
        result = ast2value(move(st)).get();
      }

      context->builder.CreateRet(result);
      return function;
    }

    template<>
    llvm::Function* makeFunction<void>(string  name, vector<shared_ptr<AST>> asts){
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














std::shared_ptr<AST> gAdd(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = llvm::make_unique<AST>("BinAddSubExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", llvm::make_unique<AST>("+"));
    return result;
}

std::shared_ptr<AST> gSub(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = llvm::make_unique<AST>("BinAddSubExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", llvm::make_unique<AST>("-"));
    return result;
}

std::shared_ptr<AST> gMul(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = llvm::make_unique<AST>("BinMulDivExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", llvm::make_unique<AST>("*"));
    return result;
}

std::shared_ptr<AST> gDiv(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = llvm::make_unique<AST>("BinMulDivExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", llvm::make_unique<AST>("/"));
    return result;
}

std::shared_ptr<AST> generateTestAst(){

  std::shared_ptr<AST> one = llvm::make_unique<AST>("1", AST::Type::Int); 
  std::shared_ptr<AST> five = llvm::make_unique<AST>("1", AST::Type::Int);
  std::shared_ptr<AST> three = llvm::make_unique<AST>("1", AST::Type::Int);
  std::shared_ptr<AST> four = llvm::make_unique<AST>("1", AST::Type::Int);
  std::shared_ptr<AST> two = llvm::make_unique<AST>("1", AST::Type::Int);

  return gAdd(move(one), gAdd(move(five), gAdd(move(three), gAdd(move(four),move(two)))));
}

int main(){

    CodeGen::init();

    auto ast = generateTestAst();
    std::vector<CodeGen::shared_ptr<AST>> asts;
    asts.push_back( move(ast) );
    CodeGen::makeFunction<int>("main", move(asts));

    CodeGen::dump();
    
    return 0;
  }