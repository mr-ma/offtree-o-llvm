#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <sstream>
#include <string>
#include <set>

using namespace llvm;

namespace {
  static cl::opt<std::string> ObfuscationOptions("obfs", cl::init(""), cl::desc("Obfuscation combination options."));
  static cl::opt<bool> NoAlwaysInline("no-always-inline", cl::init(false), cl::desc("Removes always_inline attributes."));

  struct PreparationPass : public ModulePass {
    static char ID;
    PreparationPass() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      std::set<std::string> obfsTokens;
      std::stringstream ss(ObfuscationOptions);
      std::string token;

      // Determine obfuscation tokens
      while (std::getline(ss, token, '-')) {
        if (token.find("FLAs") != std::string::npos)
          obfsTokens.insert("split");
        if (token.find("SUB") != std::string::npos)
          obfsTokens.insert("sub");
        else if (token.find("FLA") != std::string::npos)
          obfsTokens.insert("fla");
        else if (token.find("VIRT") != std::string::npos)
          obfsTokens.insert("scvirt");
        else if (token.find("BCF") != std::string::npos)
          obfsTokens.insert("bcf");
      }

      for (Function &F : M) {
        // Add obfuscation attributes
        for (std::string s : obfsTokens) {
          F.addFnAttr(s);
        }

        // Remove always_inline attribute
        if (NoAlwaysInline) {
          errs() << "Removing always_inline: " << F.getName() << "\n";
          F.removeFnAttr(Attribute::AlwaysInline);
        }
      }

      return true;
    }

  };
}

char PreparationPass::ID = 0;
static llvm::RegisterPass<PreparationPass> X("prep", "Prepares module for use in protection toolchain.");

static void registerPreparationPass(const PassManagerBuilder &,
                                    legacy::PassManagerBase &PM) {
  PM.add(new PreparationPass());
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                                             registerPreparationPass);
