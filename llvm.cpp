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
#include <memory>

#include "ast.h"

namespace CodeGen{

    using namespace std;

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

    void setEntry(llvm::Function* function){
      llvm::BasicBlock* entry = llvm::BasicBlock::Create( context->context, "entry", function);
      context->builder.SetInsertPoint(entry);
    }

    template<typename T>
    llvm::Function* makeFunction(string  name){
      return nullptr;
    }

    template<>
    llvm::Function* makeFunction<int>(string  name){
      auto function = llvm::Function::Create(
          llvm::FunctionType::get( context->builder.getInt32Ty(), false ),
          llvm::Function::ExternalLinkage,
          name,
          context->module
        );
      setEntry(function);
      return function;
    }


    template<>
    llvm::Function* makeFunction<void>(string  name){
      return  llvm::Function::Create(
          llvm::FunctionType::get( context->builder.getVoidTy(), false ),
          llvm::Function::ExternalLinkage,
          name,
          context->module
        );
    }

    unique_ptr<llvm::BinaryOperator> addExpr(unique_ptr<llvm::Value> lhs,unique_ptr<llvm::Value> rhs){
      return unique_ptr<llvm::BinaryOperator>(llvm::BinaryOperator::CreateAdd(lhs.get(),rhs.get()));
    }

    void init(){
        context = make_shared<CodeGenContext>("sharo");
    }

    template<typename T>
    unique_ptr<llvm::Value> makeValue(T v){
      throw "Not implement";
    }

    template<>
    unique_ptr<llvm::Value> makeValue<int>(int v){
      return unique_ptr<llvm::Value>(llvm::ConstantInt::get(context->context, llvm::APInt(v, 32)));
    }

    template<>
    unique_ptr<llvm::Value> makeValue<float>(float v){
      return unique_ptr<llvm::Value>(llvm::ConstantFP::get(context->context, llvm::APFloat(v)));
    }


    unique_ptr<llvm::BinaryOperator> makeBinExpr(unique_ptr<AST> ast){
        if(ast->is("BinaryExpr")){
            auto right = ast->get("right");
            auto left = ast->get("left");

            auto ope = ast->get("operator");
            if(ope->is("+")){
                return addExpr(makeValue(right->asInt()),makeValue(right->asInt()));
            }
        }
        return nullptr;
    }

};


std::unique_ptr<AST> generateTestAst(){

  std::unique_ptr<AST> one = llvm::make_unique<AST>("1", AST::Type::Int); 
  std::unique_ptr<AST> five = llvm::make_unique<AST>("5", AST::Type::Int);

  std::unique_ptr<AST> plus = llvm::make_unique<AST>("+");

  std::unique_ptr<AST> result = llvm::make_unique<AST>("BinaryExpr"); 
  result->append("right", move(one));
  result->append("left", move(five));

  result->append("operator", move(plus));

  return result;
}

int main(){

    CodeGen::init();
    auto main = CodeGen::makeFunction<int>("main");

    return 0;
  }