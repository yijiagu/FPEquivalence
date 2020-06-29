//
//  PartialDiffExecutor.h
//  Project
//
//  Created by Gu Yijia on 1/6/20.
//

#ifndef PartialDiffExecutor_h
#define PartialDiffExecutor_h

#include <unordered_map>

#include "ExecutionStateMap.h"

#include "llvm/IR/InstVisitor.h"

namespace raic{
    class PartialDiffExecutor : public llvm::InstVisitor<PartialDiffExecutor> {
    public:
        PartialDiffExecutor(llvm::Value *A, llvm::Value *B, ExecutionStateMap  &value_state_map,
                            llvm::SmallVector<std::pair<llvm::Instruction *, llvm::Value *>, 256> &inst_list):
        ori(A), modified(B),
        value_state_map_(value_state_map){
            for(auto it : inst_list){
                worklist.push_back(it.first);
                commonOpMap[it.first] = it.second;
            }
        };
        
        void run();
        
        // Visiter implementation - compute the partial difference along the common chain
        void visitFAdd(llvm::BinaryOperator &I);
        void visitFMul(llvm::BinaryOperator &I);
        void visitFSub(llvm::BinaryOperator &I);
        void visitFDiv(llvm::BinaryOperator &I);
        
        void visitCallInst(llvm::CallInst &I);
        
        /// Specify what to return for unhandled instructions.
        void visitInstruction(llvm::Instruction &I);
        
        ValueRange getTargetAbsDiff(){
            return targetAbsDiff;
        }
        
    private:
        
        AbstractState inverse(AbstractState &op);
        void inverseDiff(ValueRange oriRange);
        void multHelp(ValueRange commRange, ValueRange multRange);
        
        llvm::Value *ori;
        llvm::Value *modified;
        llvm::SmallVector<llvm::Instruction *, 256> worklist;
        std::unordered_map<llvm::Instruction *, llvm::Value *> commonOpMap;
        ExecutionStateMap &value_state_map_; //!< Map values to abstract values;
        
        AbstractState target_as;
        ValueRange targetAbsDiff;
    };

    ValueRange pd_sqrt(AbstractState &op, ValueRange targetAbsDiff);
}
#endif /* PartialDiffExecutor_h */
