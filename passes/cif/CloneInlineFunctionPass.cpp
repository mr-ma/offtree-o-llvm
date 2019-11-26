#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <string>
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Casting.h"
#include <sstream>

using namespace llvm;

namespace {
  struct CloneInlineFunctionPass : public ModulePass {
    static char ID;
    CloneInlineFunctionPass() : ModulePass(ID) {}
    void setMetadata(LLVMContext &ctx, Instruction *inst, std::string metadataStr){
      auto* guard_md_str = llvm::MDString::get(ctx, metadataStr);
      MDNode* guard_md = llvm::MDNode::get(ctx, guard_md_str);
      inst->setMetadata(metadataStr, guard_md);
    }
    void copyInstMetadataToFunc(Instruction* inst, Function* func){
      for (BasicBlock& BB:*func){
        for(Instruction& bbinst:BB){
          if(!(llvm::dyn_cast<llvm::CmpInst>(&bbinst)))
            continue;
          if(inst->getMetadata("oh_hash")){
            setMetadata(BB.getContext(), &bbinst, "oh_hash");
          } if(inst->getMetadata("oh_verify")){
            setMetadata(BB.getContext(), &bbinst, "oh_verify");
          } if(inst->getMetadata("cfi_register")){
            setMetadata(BB.getContext(), &bbinst, "cfi_register");
          } if(inst->getMetadata("cfi_verify")){
            setMetadata(BB.getContext(), &bbinst, "cfi_verify");
          } if(inst->getMetadata("sc_guard")){
            setMetadata(BB.getContext(), &bbinst, "sc_guard");
          } 
        }
      }
    }
    virtual bool runOnModule(Module &M) {
      SmallVector<Function *, 16> FunctionsToClone;
      bool Changed = false;

      errs() << "Finding Functions to Clone\n";
      for (Function &F : M) {
        if (!F.isDeclaration() && F.hasFnAttribute(Attribute::AlwaysInline) && isInlineViable(F)) {
          errs() << "\t" << F.getName() << "\n";
          FunctionsToClone.push_back(&F);
        }
      }

      errs() << "\nCloning Functions\n";
      for (Function *F : FunctionsToClone) {
        errs() << "\t" << F->getName() << "\n";

        // Find Call sites
        for (User *U : F->users()) {
          if (auto CS = CallSite(U)) {
            if (CS.getCalledFunction() == F) {
              ValueToValueMapTy VMap;
              errs() << "\t\t" << CS.getCaller()->getName() << "\n";
              Function *cloned = CloneFunction(F, VMap);
              //copy instruction metadata
              copyInstMetadataToFunc(CS.getInstruction(), cloned);
              errs() << "\t\t" << cloned->getType()->isPointerTy() << cloned->getType()->isFunctionTy() << "\n";
              CS.setCalledFunction(cloned);
              Changed = true;
            }
            else
              errs() << "\t\t--" << CS.getCaller()->getName() << "\n";
          }
        }

        // Delete old Function
        //M.getFunctionList().erase(F);
      }
      errs() << "\nDone\n";

      return Changed;
    }

  };
}

char CloneInlineFunctionPass::ID = 0;
static llvm::RegisterPass<CloneInlineFunctionPass> X("cif", "Clones inlined functions");

static void registerCloneInlineFunctionPass(const PassManagerBuilder &,
                                            legacy::PassManagerBase &PM) {
  PM.add(new CloneInlineFunctionPass());
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                                             registerCloneInlineFunctionPass);
