//===- InstCombine.h - InstCombine pass -------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file provides the primary interface to the instcombine pass. This pass
/// is suitable for use in the new pass manager. For a pass that works with the
/// legacy pass manager, please look for \c createAnnoInstructionCombiningPass() in
/// Scalar.h.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_LOOPVECTORIZE_H
#define LLVM_TRANSFORMS_LOOPVECTORIZE_H

#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/DemandedBits.h"
#include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include <functional>

namespace raic{
    struct LoopVectorizePass : public llvm::PassInfoMixin<LoopVectorizePass> {
        bool DisableUnrolling = false;
        /// If true, consider all loops for vectorization.
        /// If false, only loops that explicitly request vectorization are
        /// considered.
        bool AlwaysVectorize = true;
        
        llvm::ScalarEvolution *SE;
        llvm::LoopInfo *LI;
        llvm::TargetTransformInfo *TTI;
        llvm::DominatorTree *DT;
        llvm::BlockFrequencyInfo *BFI;
        llvm::TargetLibraryInfo *TLI;
        llvm::DemandedBits *DB;
        llvm::AliasAnalysis *AA;
        llvm::AssumptionCache *AC;
        std::function<const llvm::LoopAccessInfo &(llvm::Loop &)> *GetLAA;
        llvm::OptimizationRemarkEmitter *ORE;
        
        llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
        
        // Shim for old PM.
        bool runImpl(llvm::Function &F, llvm::ScalarEvolution &SE_, llvm::LoopInfo &LI_,
                     llvm::TargetTransformInfo &TTI_, llvm::DominatorTree &DT_,
                     llvm::BlockFrequencyInfo &BFI_, llvm::TargetLibraryInfo *TLI_,
                     llvm::DemandedBits &DB_, llvm::AliasAnalysis &AA_, llvm::AssumptionCache &AC_,
                     std::function<const llvm::LoopAccessInfo &(llvm::Loop &)> &GetLAA_,
                     llvm::OptimizationRemarkEmitter &ORE);
        
        bool processLoop(llvm::Loop *L);
    };

    /// The LoopVectorize Pass.
    struct LoopVectorize : public llvm::FunctionPass {
        /// Pass identification, replacement for typeid
        static char ID;
        
        explicit LoopVectorize(bool NoUnrolling = false, bool AlwaysVectorize = true)
        : FunctionPass(ID) {
            Impl.DisableUnrolling = NoUnrolling;
            Impl.AlwaysVectorize = AlwaysVectorize;
            initializeLoopVectorizePass(*llvm::PassRegistry::getPassRegistry());
        }
        
        LoopVectorizePass Impl;
        bool runOnFunction(llvm::Function &F) override{
            if (skipFunction(F))
                return false;
            auto *SE = &getAnalysis<llvm::ScalarEvolutionWrapperPass>().getSE();
            auto *LI = &getAnalysis<llvm::LoopInfoWrapperPass>().getLoopInfo();
            
            auto *TTI = &getAnalysis<llvm::TargetTransformInfoWrapperPass>().getTTI(F);
            auto *DT = &getAnalysis<llvm::DominatorTreeWrapperPass>().getDomTree();
            auto *BFI = &getAnalysis<llvm::BlockFrequencyInfoWrapperPass>().getBFI();
            auto *TLIP = getAnalysisIfAvailable<llvm::TargetLibraryInfoWrapperPass>();
            auto *TLI = TLIP ? &TLIP->getTLI() : nullptr;
            auto *AA = &getAnalysis<llvm::AAResultsWrapperPass>().getAAResults();
            auto *AC = &getAnalysis<llvm::AssumptionCacheTracker>().getAssumptionCache(F);
            auto *LAA = &getAnalysis<llvm::LoopAccessLegacyAnalysis>();
            auto *DB = &getAnalysis<llvm::DemandedBitsWrapperPass>().getDemandedBits();
            auto *ORE = &getAnalysis<llvm::OptimizationRemarkEmitterWrapperPass>().getORE();
            
            std::function<const llvm::LoopAccessInfo &(llvm::Loop &)> GetLAA =
            [&](llvm::Loop &L) -> const llvm::LoopAccessInfo & { return LAA->getInfo(&L); };
            
            return Impl.runImpl(F, *SE, *LI, *TTI, *DT, *BFI, TLI, *DB, *AA, *AC,
                                GetLAA, *ORE);
        }
        void getAnalysisUsage(llvm::AnalysisUsage &AU) const override{
            AU.addRequired<llvm::AssumptionCacheTracker>();
            AU.addRequired<llvm::BlockFrequencyInfoWrapperPass>();
            AU.addRequired<llvm::DominatorTreeWrapperPass>();
            AU.addRequired<llvm::LoopInfoWrapperPass>();
            AU.addRequired<llvm::ScalarEvolutionWrapperPass>();
            AU.addRequired<llvm::TargetTransformInfoWrapperPass>();
            
            AU.addRequired<llvm::AAResultsWrapperPass>();
            AU.addRequired<llvm::LoopAccessLegacyAnalysis>();
            AU.addRequired<llvm::DemandedBitsWrapperPass>();
            AU.addRequired<llvm::OptimizationRemarkEmitterWrapperPass>();
            AU.addPreserved<llvm::LoopInfoWrapperPass>();
            AU.addPreserved<llvm::DominatorTreeWrapperPass>();
            AU.addPreserved<llvm::BasicAAWrapperPass>();
            AU.addPreserved<llvm::GlobalsAAWrapperPass>();
        }
    };
}

#endif
