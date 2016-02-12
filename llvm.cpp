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

namespace codegen{

template<typename T>
    struct CodeGenContext{
        llvm::LLVMContext context;
        llvm::Module const module;

        CodeGenContext(string moduleName){
            context = llvm::getGlobalContext();
        }


    }; 

    shared_ptr<CodeDenContext> context; 

    llvm::Function* makeFunction<void>(string  name){
      return  llvm::Function::Create(
          llvm::FunctionType::get( builder.getVoidTy(), false ),
          llvm::Function::ExternalLinkage,
          name,
          module.get()
        );
    }

    template<typename T>
    llvm::Function* makeFunction(){
      return nullptr;
    }

}
int main(){


    return 0;
  }