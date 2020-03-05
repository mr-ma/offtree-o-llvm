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

using namespace llvm;

namespace {
  struct DeleteInlineFunctionPass : public ModulePass {
    static char ID;
    DeleteInlineFunctionPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      InlineFunctionInfo IFI;
      SmallSetVector<CallSite, 16> Calls;
      SmallVector<Function *, 16> FunctionsToRemove;
      bool Changed = false;

      for (Function &F : M) {
        if (!F.isDeclaration() && F.hasFnAttribute(Attribute::AlwaysInline) && isInlineViable(F)) {
          errs() << F.getName() << " has inline attr\n";
          Calls.clear();

          for (User *U : F.users()) {
            if (auto CS = CallSite(U)) {
              if (CS.getCalledFunction() == &F) {
                errs() << "\t" << CS.getCaller()->getName() << "\n";
                Calls.insert(CS);
              }
            }
          }

          for (CallSite CS : Calls)
            Changed |= InlineFunction(CS, IFI, nullptr, false);

          FunctionsToRemove.push_back(&F);          
        }
      }

      // Delete functions
      errs() << "Deleting functions\n";
      for (Function *F : FunctionsToRemove) {
        errs() << "\t" << F->getName() << "\n";
        /*F->dropAllReferences();
        errs() << "Dropped references\n";
        F->removeFromParent();
        errs() << "Removed from parent\n";
        F->deleteBody();
        errs() << "Deleted Body\n";*/
        M.getFunctionList().erase(F);
      }
      errs() <<"DONE\n";
      return Changed;
    }
  };
}

char DeleteInlineFunctionPass::ID = 0;
static llvm::RegisterPass<DeleteInlineFunctionPass> X("dif", "Removes inlined functions");

static void registerDeleteInlineFunctionPass(const PassManagerBuilder &,
                                             legacy::PassManagerBase &PM) {
  PM.add(new DeleteInlineFunctionPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerDeleteInlineFunctionPass);
