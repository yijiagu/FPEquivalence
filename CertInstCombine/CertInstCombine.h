//
//  CertInstCombine.h
//  LLVM
//
//  Created by Gu Yijia on 10/16/17.
//

#ifndef CERT_INSTCOMBINE_H
#define CERT_INSTCOMBINE_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/CommandLine.h"

//#include "../AnnoInstCombine/AnnoInstCombine.h"
#include "../LoopVectorize/LoopVectorize.h"
#include "Executor.h"
#include "SplitStrategy.h"

#include <queue>

namespace raic{
    
    class CertInstCombinePass : public llvm::PassInfoMixin<CertInstCombinePass> {
        
    public:
        static char ID; // Pass identification, replacement for typeid
        static llvm::StringRef name() { return "CertInstCombinePass"; }
        
        explicit CertInstCombinePass(){}
        
        llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);
    };
    
    class CertInstCombine : public llvm::FunctionPass {
    public:
        static char ID; // Pass identification, replacement for typeid
        std::set<llvm::Value *> VSet;
        
        CertInstCombine() : llvm::FunctionPass(ID) {}
        bool runOnFunction(llvm::Function &F) override;
        
        void getAnalysisUsage(llvm::AnalysisUsage &AU) const override{
            //AU.addRequired<llvm::AnnoInstructionCombiningPass>();
            //AU.addRequired<LoopVectorize>();
            AU.setPreservesAll ();
        }
    private:
        ValueRange getErrorBound(const std::vector<ValueRange> &input_ranges, llvm::Function &F);
        
    private:
        double f_rel_tol = 0.07;
        double f_abs_tol = 0.0000000000000001;
        double max_iters = 5000; //max number of iterations
        double x_tol = 0.000000000000000001; //

        std::queue<SplitStrategy> ss_queue_;
    };
    
    struct DoubleInterval{
        double lower_bound;
        double upper_bound;
    };
    
    struct IntervalParser : public llvm::cl::basic_parser<DoubleInterval> {
        IntervalParser(llvm::cl::Option &O) : llvm::cl::basic_parser<DoubleInterval>(O) {}
        // parse - Return true on error.
        bool parse(llvm::cl::Option &O, llvm::StringRef ArgName, const std::string &ArgValue,
                   DoubleInterval &Val);
    };
    
}
#endif /* CertInstCombine_h */

