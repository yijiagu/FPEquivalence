//
//  StableInstCombinePass.cpp
//  ALL_BUILD
//
//  Created by Gu Yijia on 6/25/18.
//

#include "llvm/Pass.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"


#include "../CertInstCombine/CertInstCombine.h"

#include <iostream>


using namespace llvm;
using namespace raic;

cl::opt<std::string> TargetFunction("func", cl::desc("Specify the target function for analysis"), cl::value_desc ("functionname"));

namespace {
    struct StableInstCombinePass : public ModulePass {
    public:
        static char ID;
        StableInstCombinePass() : ModulePass(ID) {}
        bool disableFastMath(Function &F, DenseMap<const Value*, Value*> &VMap);
        virtual bool runOnModule(Module &M) override {
            auto FPMCombine = legacy::FunctionPassManager(&M);
            
            FPMCombine.add(new InstructionCombiningPass());
            
            for(Function &F : M){
                if(!F.getName().contains(TargetFunction)){
                    continue;
                }
                bool isSuccess = false;
                Function *cloneF;
                Function *fp = &F;

                ValueToValueMapTy VMap;
                cloneF = llvm::CloneFunction(fp, VMap);
                DenseMap<const Value*, Value*> denseVMap;
                for(auto i: VMap){
                    denseVMap.insert(std::make_pair(i.first, i.second));
                }
                isSuccess = disableFastMath(F, denseVMap);

                std::string fname = F.getName().str();
                for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
                    I -> replaceAllUsesWith(UndefValue::get(I -> getType()));
                for (Function::arg_iterator ai = F.arg_begin(), ai_e = F.arg_end(); ai != ai_e; ++ai){
                    ai->replaceAllUsesWith(UndefValue::get(ai -> getType()));
                }
                
                F.eraseFromParent();
                
                cloneF -> setName(fname);
                if(isSuccess){
                    FPMCombine.run(*cloneF);
                }
                break;
            }
            return false;
        }
        
        void getAnalysisUsage(llvm::AnalysisUsage &AU) const override{
            AU.addRequired<CertInstCombine>();
            AU.setPreservesAll();
        }
    };
}
bool StableInstCombinePass::disableFastMath(llvm::Function &F, DenseMap<const Value*, Value*> &denseVMap){
    CertInstCombine& cert_instcombine = getAnalysis<CertInstCombine>(F);
    
    bool isSuccess = true;
    for(auto v : cert_instcombine.VSet){
        if(denseVMap.find(v) != denseVMap.end()){
            if(Instruction *I = dyn_cast<Instruction> (denseVMap[v])){
                I->print(dbgs());
                FastMathFlags mathFlag;
                I->copyFastMathFlags(mathFlag);
            }
        }else{
            Instruction *I = dyn_cast<Instruction> (v);
             I->print(dbgs());
            isSuccess = false;
        }
    }
    return isSuccess;
}
char StableInstCombinePass::ID = 0;
static RegisterPass<StableInstCombinePass> X("stableinstcombine", "Stable InstCombine Pass");
