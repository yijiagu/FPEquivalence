//
//  PerturbationExecutor.h
//  Project
//
//  Created by Gu Yijia on 12/5/17.
//

#ifndef Executor_h
#define Executor_h

#include "../AnnoInstCombine/Perturbation.h"
#include "ExecutionStateMap.h"

#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"


#include <map>

namespace raic{
    using Edge = std::pair<llvm::BasicBlock*,llvm::BasicBlock*>; //!< CFG edge.
 
    /// Class to implement the logic of interval analysis and automatic differentiation
    //TODO: handle global variable
    class Executor : public llvm::InstVisitor<Executor> {
    public:
        Executor(llvm::Function *fp, ExecutionStateMap  &value_state_map,
                 llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet):fp_(fp),
        value_state_map_(value_state_map){}
        
        void run();
        void runPartialDifference();
        
    //private:
        //implement integer type operations
        void visitSExt(llvm::SExtInst &I);
        void visitShl(llvm::BinaryOperator &I);
        void visitAdd(llvm::BinaryOperator &I);
        void visitSub(llvm::BinaryOperator &I);
        void visitMul(llvm::BinaryOperator &I);
        
        void visitOr(llvm::BinaryOperator &I);
        void visitAnd(llvm::BinaryOperator &I);
    
        
        // Visitation implementation - Implement interval analysis and automatic differentiation for floating-point
        // instruction types.  The semantics are as follows:
        void visitFAdd(llvm::BinaryOperator &I);
        void visitFMul(llvm::BinaryOperator &I);
        void visitFSub(llvm::BinaryOperator &I);
        void visitFDiv(llvm::BinaryOperator &I);
        
        void visitGetElementPtrInst (llvm::GetElementPtrInst &I);
        void visitStoreInst(llvm::StoreInst &SI);
        void visitLoadInst(llvm::LoadInst &LI);
        void visitAllocaInst(llvm::AllocaInst &I);
        
        void visitExtractElementInst (llvm::ExtractElementInst &I);
        void visitInsertElementInst (llvm::InsertElementInst &I);
        void visitShuffleVectorInst (llvm::ShuffleVectorInst &I);
        
        void visitIntrinsic(llvm::IntrinsicInst &II);
        
        void visitReturnInst(llvm::ReturnInst &I);
        
        void visitBranchInst(llvm::BranchInst &I);
        void visitICmpInst(llvm::ICmpInst &I);
        void visitFCmpInst(llvm::FCmpInst &I);
        void visitPHINode(llvm::PHINode       &I);
        void visitSwitchInst(llvm::SwitchInst  &I);
        void visitSIToFP(llvm::SIToFPInst &I);
        
        void visitBitCastInst(llvm::BitCastInst &I);
        void visitZExtInst(llvm::ZExtInst &I);
        void visitSelectInst(llvm::SelectInst &I);
        
        /// Specify what to return for unhandled instructions.
        void visitInstruction(llvm::Instruction &I) {
            assert(false && "visit unsupported instruction");
        };
        
        void visitCallInst(llvm::CallInst &I);
        
        void addBlockInstToWorklist(llvm::BasicBlock *BB);
        
        AbstractState getTargetAbstractState(){
            return targetAs;
        }
        
        ValueRange getTargetAbsDiff(){ // return the absolute difference
            return targetAbsDiff;
        }
        
        /* If the target total range does not contain 0, return the degree of approximation - epsilon;
           otherwise return the absolute difference
         */
        ValueRange getTargetEpsilon(){
            ValueRange total_vr = targetAs.getTotalRange();
            if(total_vr.lower() <= 0 && total_vr.upper() >= 0){
                return targetAbsDiff;
            }
            ValueRange abs_total_vr = abs(total_vr);
            
            return targetAbsDiff;
            
//            return targetAbsDiff/abs_total_vr;
            
        }

    private:
        int getAbstractSize(llvm::Type *ty);
        PointerIndex computeGEP(llvm::GEPOperator *gep);
        
        //find the max common chain between original and modified program
        std::pair<llvm::Value*, llvm::Value*> findMaxCommonChain(llvm::Value *A, llvm::Value *B, llvm::SmallVector<std::pair<llvm::Instruction *, llvm::Value*>, 256> &inst_list);
        //iterate the function; build the instruction list for the original program; find the instruction responsible for computing the difference
        std::pair<llvm::Value*, llvm::Value*> findPartialDiffInstr(llvm::BasicBlock *BB, llvm::SmallVector<llvm::Instruction *, 256>  &inst_list);
        //build the instruction list for the difference in the modified program
        llvm::SmallVector<llvm::Instruction *, 256> buildInstListOfDiff(llvm::Value* A);
        
        llvm::Function *fp_; //!< The function where the analysis lives
        ExecutionStateMap &value_state_map_; //!< Map values to abstract values
        llvm::SmallVector<llvm::Instruction *, 256> worklist;
        Edge  curr_executable_edge_;  //!< current executable edge.
        
        AbstractState targetAs;
        ValueRange targetAbsDiff;
        
        //handle reduction optimization
        AbstractState *accumulated = new AbstractState();
        llvm::Value* reduce_I = nullptr;
    };
}
#endif /* Executor_h */
