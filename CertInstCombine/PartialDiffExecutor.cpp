//
//  PartialDiffExecutor.cpp
//  LLVMCertInstCombine
//
//  Created by Gu Yijia on 1/6/20.
//
#include "PartialDiffExecutor.h"

#include <iostream>

#include "llvm/Support/Debug.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"

using namespace std;
using namespace llvm;
using namespace raic;

#define DEBUG_TYPE "certinstcombine"

void PartialDiffExecutor::run(){
    AbstractState *abstract_ori = value_state_map_.atAbstractState(ori);
    AbstractState *abstract_modified = value_state_map_.atAbstractState(modified);
    
    target_as.taylor_form = abstract_ori -> taylor_form;
    for(auto i : abstract_modified -> taylor_form){
         target_as.taylor_form[i.first] = target_as.taylor_form[i.first] - i.second;
    }
    
    targetAbsDiff = ValueRange(target_as.getMinError(), target_as.getMaxError());

    for(auto I : worklist){
        DEBUG(dbgs() << "visit inst:" << *I << "\n");
        visit(*I);
    }
}

void PartialDiffExecutor::visitFAdd(BinaryOperator &I){
    Value* op0 = I.getOperand(0);
    Value* op1 = I.getOperand(1);
    
    auto eps = ldexp(1.0, -53);
     
    ValueRange m1 = targetAbsDiff * (1 + eps);
    
    AbstractState *as0 = value_state_map_.atAbstractState(op0);
    AbstractState *as1 = value_state_map_.atAbstractState(op1);
    ValueRange addRange = as0 -> getTotalRange() + as1 -> getTotalRange();
    ValueRange m3 = 2 * eps * abs(addRange);
    targetAbsDiff = m1 + m3;
}

void PartialDiffExecutor::visitFMul(BinaryOperator &I){
    Value* op0 = I.getOperand(0);
    Value* op1 = I.getOperand(1);
    
    if(commonOpMap[&I] == op0){
        AbstractState *commAs = value_state_map_.atAbstractState(op0);
        AbstractState *diffAs = value_state_map_.atAbstractState(op1);
        ValueRange multRange = commAs -> getTotalRange() * (diffAs -> getTotalRange());
        multHelp(commAs -> getTotalRange(), multRange);
        return;
    }else{
        AbstractState *commAs = value_state_map_.atAbstractState(op1);
        AbstractState *diffAs = value_state_map_.atAbstractState(op0);
        ValueRange multRange = commAs -> getTotalRange() * (diffAs -> getTotalRange());
        multHelp(commAs -> getTotalRange(), multRange);
        return;
    }
    
    
    assert(false && "Cannot find common operator for FMult");
}

void PartialDiffExecutor::visitFSub(BinaryOperator &I){
    
}

void PartialDiffExecutor::visitFDiv(BinaryOperator &I){
    Value* op0 = I.getOperand(0);
    Value* op1 = I.getOperand(1);
    if(commonOpMap[&I] == op0){
        AbstractState *oriAs = value_state_map_.atAbstractState(op1);
        AbstractState inverseAs = inverse(*oriAs);
        
        inverseDiff(oriAs -> getTotalRange());
        
        AbstractState *commAs = value_state_map_.atAbstractState(op0);
        ValueRange multRange = inverseAs.getTotalRange() * commAs -> getTotalRange();
        multHelp(commAs -> getTotalRange(), multRange);
        return;
    }else if(commonOpMap[&I] == op1){
        AbstractState *commAs = value_state_map_.atAbstractState(op1);
        AbstractState *diffAs = value_state_map_.atAbstractState(op0);
        AbstractState inverseAs = inverse(*commAs);
        ValueRange multRange = diffAs -> getTotalRange() * inverseAs.getTotalRange();
        multHelp(inverseAs.getTotalRange(), multRange);
        return;
    }
    
    assert(false && "Cannot find common operator for FDiv");
}

AbstractState PartialDiffExecutor::inverse(AbstractState &op){ //copied from AbstractState.cpp
    AbstractState r;
    r.setValueRange(1.0 / op.getValueRange());
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = tr * tr * tr;
    if(d.lower() <= 0 && d.upper() >= 0){
    assert(false && "Division by zero");
    }
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    
    auto b_high = abs(1.0 / d).upper();
    auto m3 = b_high * error_mult;
    
    ValueRange m1 = -1.0/(op.getValueRange() * op.getValueRange());
    for(auto iter : op.taylor_form){
    r.taylor_form[iter.first] = r.taylor_form[iter.first] + iter.second.multValueRange(m1);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);


    return r;
}

void PartialDiffExecutor::inverseDiff(ValueRange oriRange){
    ValueRange tr = oriRange + ValueRange(-targetAbsDiff.upper(), targetAbsDiff.upper());
    ValueRange d = tr * tr * tr;
    
    if(d.lower() <= 0 && d.upper() >= 0){
       assert(false && "Division by zero");
    }
    
    ValueRange diff_mult = targetAbsDiff * targetAbsDiff;
    auto b_high = abs(1.0 / d).upper();
    auto m3 = b_high * diff_mult;
    
    ValueRange m1 = abs(-1.0/(oriRange * oriRange)) * targetAbsDiff;
    targetAbsDiff = m1 + m3;
    
}

//implement sqrt
ValueRange raic::pd_sqrt(AbstractState &op, ValueRange targetAbsDiff){
     ValueRange tr = op.getTotalRange() + ValueRange(-targetAbsDiff.upper(), targetAbsDiff.upper());
     ValueRange d = sqrt(tr * tr * tr);
   
    
    ValueRange diff_mult = targetAbsDiff * targetAbsDiff;
    auto b_high = (0.125 * abs(d)).upper();
    auto m3 = b_high * diff_mult;
    
    ValueRange m1 = abs(targetAbsDiff/(2.0 * sqrt(op.getTotalRange())));
    ValueRange r = m1 + m3;
    
    auto eps = ldexp(1.0, -53);
    r = r * (1 + eps);
    r = r + 2 * eps * abs(sqrt(op.getTotalRange()));
    
    return r;
   
}

void PartialDiffExecutor::multHelp(ValueRange commRange, ValueRange multRange){
    auto eps = ldexp(1.0, -53);
    
    ValueRange m1 = targetAbsDiff * abs(commRange) * (1 + eps);
    ValueRange m3 = 2 * eps * abs(multRange);
    targetAbsDiff = m1 + m3;
}





void PartialDiffExecutor::visitCallInst(CallInst &I){
    StringRef fnName = I.getCalledFunction()->getName();

    if(fnName == "llvm.dbg.declare" || fnName == "llvm.dbg.value"){
    return;
    }

    Value *A = I.getOperand(0);
    AbstractState *abstract_A = value_state_map_.atAbstractState(A);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);

    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();


    if(fnName == "sin") {
    *abstract_I = sin(*abstract_A);
    //ExecutionStateCalculation::updateSinExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "exp"){
    *abstract_I = exp(*abstract_A);
    //ExecutionStateCalculation::updateExpExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "cos") {
    *abstract_I = cos(*abstract_A);
    //ExecutionStateCalculation::updateCosExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "tan") {
    *abstract_I = tan(*abstract_A);
    //ExecutionStateCalculation::updateTanExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "atan2"){
    Value *B = I.getOperand(1);
    AbstractState *abstract_B = value_state_map_.atAbstractState(B);

    *abstract_I = atan2(*abstract_A, *abstract_B);

    //ExecutionStateCalculation::updateAtan2ExecutionState(&I, I.getOperand(0), I.getOperand(1), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "asin"){
    *abstract_I = asin(*abstract_A);
    //ExecutionStateCalculation::updateAsinExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "log"){
    *abstract_I = log(*abstract_A);
    //ExecutionStateCalculation::updateLogExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "llvm.sqrt.f64"){
     targetAbsDiff = pd_sqrt(*abstract_A, targetAbsDiff);
    //ExecutionStateCalculation::updateSqrtExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else {
    assert(false && "Calling Unsupported Function");
    }
}


