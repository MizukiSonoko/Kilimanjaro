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

    llvm::Value* add(llvm::Value* lhs,llvm::Value* rhs){
      return llvm::BinaryOperator::CreateAdd(move(lhs), move(rhs));
    }

    llvm::Value* sub(llvm::Value* lhs,llvm::Value* rhs){
      return llvm::BinaryOperator::CreateSub(move(lhs), move(rhs));
    }

    llvm::Value* mul(llvm::Value* lhs,llvm::Value* rhs){
      return llvm::BinaryOperator::CreateMul(move(lhs), move(rhs));
    }


    void init(){
        context = make_shared<CodeGenContext>("sharo");
    }

};

int main(){

    CodeGen::init();
    auto main = CodeGen::makeFunction<int>("main");

    return 0;
  }