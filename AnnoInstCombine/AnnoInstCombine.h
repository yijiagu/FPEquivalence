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

#ifndef LLVM_TRANSFORMS_INSTCOMBINE_ANNOINSTCOMBINE_H
#define LLVM_TRANSFORMS_INSTCOMBINE_ANNOINSTCOMBINE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/InstCombine/InstCombineWorklist.h"
#include "llvm/ADT/SmallSet.h"

#include "Perturbation.h"

namespace llvm {
    
    class AnnoInstCombinePass : public PassInfoMixin<AnnoInstCombinePass> {
        InstCombineWorklist Worklist;
        bool ExpensiveCombines;
        
    public:
        static StringRef name() { return "AnnoInstCombinePass"; }
        PerturbationMap perturbation_map_;
        
        explicit AnnoInstCombinePass(bool ExpensiveCombines = true)
        : ExpensiveCombines(ExpensiveCombines) {}
        
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
    };
    
    /// \brief The legacy pass manager's instcombine pass.
    ///
    /// This is a basic whole-function wrapper around the instcombine utility. It
    /// will try to combine all instructions in the function.
    class AnnoInstructionCombiningPass : public FunctionPass {
        InstCombineWorklist Worklist;
        const bool ExpensiveCombines;
        
    public:
        static char ID; // Pass identification, replacement for typeid
        PerturbationMap perturbation_map_;
        
        AnnoInstructionCombiningPass(bool ExpensiveCombines = true)
        : FunctionPass(ID), ExpensiveCombines(ExpensiveCombines) {
            initializeInstructionCombiningPassPass(*PassRegistry::getPassRegistry());
        }
        
        void getAnalysisUsage(AnalysisUsage &AU) const override;
        bool runOnFunction(Function &F) override;
    };
}

#endif
