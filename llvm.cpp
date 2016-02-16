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

    unique_ptr<llvm::BinaryOperator> addExpr(unique_ptr<llvm::Value> lhs,unique_ptr<llvm::Value> rhs){
      return unique_ptr<llvm::BinaryOperator>(llvm::BinaryOperator::CreateAdd(lhs.get(),rhs.get()));
    }

    void init(){
        context = make_shared<CodeGenContext>("sharo");
    }

    template<typename T>
    unique_ptr<llvm::Value> makeValue(T v){
      throw ValueException("Not implement");
    }

    template<>
    unique_ptr<llvm::Value> makeValue<int>(int v){
      bool  is_signed = false;
      return unique_ptr<llvm::Value>(llvm::ConstantInt::get(context->context, llvm::APInt(/*nbits*/32, v, /*bool*/is_signed)));
    }

    template<>
    unique_ptr<llvm::Value> makeValue<float>(float v){
      return unique_ptr<llvm::Value>(llvm::ConstantFP::get(context->context, llvm::APFloat(v)));
    }

    template<>
    unique_ptr<llvm::Value> makeValue<double>(double v){
      return unique_ptr<llvm::Value>(llvm::ConstantFP::get(context->context, llvm::APFloat(v)));
    }


    void makeBinExpr(unique_ptr<AST> ast){
        if(ast->is("BinaryExpr")){
            auto right = ast->get("right");
            auto left = ast->get("left");
            if(ast->get("operator")->is("+")){
                context->builder.CreateFAdd(
                  makeValue(right->asFloat()).get(),
                  makeValue(left->asFloat()).get(),
                  "addtmp"
                );
                return;
            }
        }
        throw ValueException("You must insert correct BinExpr");
    }


    void setEntry(llvm::Function* function){
      llvm::BasicBlock* entry = llvm::BasicBlock::Create( context->context, "entry", function);
      context->builder.SetInsertPoint(entry);
    }

    template<typename T>
    llvm::Function* makeFunction(string  name, vector<unique_ptr<AST>> asts){
      return nullptr;
    }

    template<>
    llvm::Function* makeFunction<int>(string  name, vector<std::unique_ptr<AST>> asts){
      auto function = llvm::Function::Create(
          llvm::FunctionType::get( context->builder.getInt32Ty(), false ),
          llvm::Function::ExternalLinkage,
          name,
          context->module
        );
      setEntry(function);


      auto ret = context->builder.CreateNUWAdd(
        makeValue(1).get(),
        makeValue(3).get(),
        "addtmp"
      );

/*
      for(auto& ast : asts){
        makeBinExpr(move(ast));
      }
*/
      context->builder.CreateRet(ret);

      return function;
    }


    template<>
    llvm::Function* makeFunction<void>(string  name, vector<unique_ptr<AST>> asts){
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


std::unique_ptr<AST> generateTestAst(){

  std::unique_ptr<AST> one = llvm::make_unique<AST>("1.0", AST::Type::Float); 
  std::unique_ptr<AST> five = llvm::make_unique<AST>("5.0", AST::Type::Float);

  std::unique_ptr<AST> plus = llvm::make_unique<AST>("+");

  std::unique_ptr<AST> result = llvm::make_unique<AST>("BinaryExpr"); 
  result->append("right", move(one));
  result->append("left", move(five));

  result->append("operator", move(plus));

  return result;
}

int main(){

    CodeGen::init();

    auto ast = generateTestAst();
    std::vector<CodeGen::unique_ptr<AST>> asts;
    asts.push_back( move(ast) );
    CodeGen::makeFunction<int>("main", move(asts));

    CodeGen::dump();
    
    return 0;
  }