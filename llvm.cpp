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

    llvm::Value* ast2value(shared_ptr<AST> ast);
    
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

    llvm::Value* makeBinMulDivExpr(shared_ptr<AST> ast){
        auto right = ast->get("right");
        auto left  = ast->get("left");
        auto ope   = ast->get("operator");
        if(ope->is("*")){
            return context->builder.CreateMul(
               ast2value(move(right)),
               ast2value(move(left)),
              "multmp"
            );
        }else if(ope->is("/")){
            return context->builder.CreateUDiv(
               ast2value(move(right)),
               ast2value(move(left)),
              "divtmp"
            );
        }
        throw ValueException("Not implement makeBinMulDivExpr");
    }
    
    llvm::Value* makeBinAddSubExpr(shared_ptr<AST> ast){
        auto right = ast->get("right");
        auto left  = ast->get("left");
        auto ope   = ast->get("operator");
        if(ope->is("+")){
            return context->builder.CreateAdd(
               ast2value(move(right)),
               ast2value(move(left)),
              "addtmp"
            );
        }else if(ope->is("-")){
            return context->builder.CreateSub(
               ast2value(move(right)),
               ast2value(move(left)),
              "subtmp"
            );
        }
        throw ValueException("Not implement makeBinAddSubExpr");
    }

    void VariableDecl(shared_ptr<AST> ast){
        auto value = ast->get("Value");
        auto expr  = ast->get("Expr");
        // 上書きは今回認められないわぁ
        if(context->NameTable.find(value->name()) != context->NameTable.end()){      
            throw ValueException(value->name() + " is already defined!");
        }
        context->NameTable[value->name()] = ast2value(move(expr));
    }

    llvm::Value* getValue(shared_ptr<AST> ast){
        auto name = ast->valueName();
        if(context->NameTable.find(name) != context->NameTable.end())
            return context->NameTable[name];
        throw ValueException(name+" is undefined value");
    }


    llvm::Value* ast2value(shared_ptr<AST> ast){
        if(ast->isInt()){
            return makeValue(ast->asInt());
        }else if(ast->isFloat()){
            return makeValue(ast->asFloat());
        }else if(ast->is("BinAddSubExpr")){
            return makeBinAddSubExpr(move(ast));
        }else if(ast->is("BinMulDivExpr")){
            return makeBinMulDivExpr(move(ast));
        }else if(ast->is("Value")){
            return getValue(move(ast));
        }else if(ast->is("VariableDecl")){
            VariableDecl(move(ast));
            return nullptr;
        }else{
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
        result = ast2value(move(st));
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







std::shared_ptr<AST> gValDecl(std::string name, std::shared_ptr<AST> expr){
    std::shared_ptr<AST> result = CodeGen::make_shared<AST>("VariableDecl");
    result->append("Value", CodeGen::make_shared<AST>(name));
    result->append("Expr", move(expr));
    return result;
}

std::shared_ptr<AST> gAdd(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = CodeGen::make_shared<AST>("BinAddSubExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST>("+"));
    return result;
}

std::shared_ptr<AST> gSub(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = CodeGen::make_shared<AST>("BinAddSubExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST>("-"));
    return result;
}

std::shared_ptr<AST> gMul(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = CodeGen::make_shared<AST>("BinMulDivExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST>("*"));
    return result;
}

std::shared_ptr<AST> gDiv(std::shared_ptr<AST> l,std::shared_ptr<AST> r){
    std::shared_ptr<AST> result = CodeGen::make_shared<AST>("BinMulDivExpr"); 
    result->append("right", move(r));
    result->append("left", move(l));
    result->append("operator", CodeGen::make_shared<AST>("/"));
    return result;
}

std::vector<CodeGen::shared_ptr<AST>> generateTestAst(){
  std::vector<CodeGen::shared_ptr<AST>> result;
  
  // val hoge = 1 + 2;
  std::shared_ptr<AST> one = CodeGen::make_shared<AST>("1", AST::Type::Int); 
  std::shared_ptr<AST> two = CodeGen::make_shared<AST>("2", AST::Type::Int);
  result.push_back( gValDecl("hoge", gAdd(move(one), move(two))));

  // val huga = hoge + 3;
  std::shared_ptr<AST> three = CodeGen::make_shared<AST>("3", AST::Type::Int);
  std::shared_ptr<AST> hoge = CodeGen::make_shared<AST>("Value","hoge");
  result.push_back( gValDecl("huga", gMul(move(three), hoge)));

  // hoge + huga
  result.push_back( gAdd(move(hoge), CodeGen::make_shared<AST>("Value","huga")));
  return result;
}

int main(){

    CodeGen::init();

    auto ast = generateTestAst();
    CodeGen::makeFunction<int>("main", move(ast));

    CodeGen::dump();
    
    return 0;
  }