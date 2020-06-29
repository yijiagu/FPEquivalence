//
//  PerturbationExecutor.cpp
//  LLVMCertInstCombine
//
//  Created by Gu Yijia on 12/5/17.
//

#include "PerturbationExecutor.h"

using namespace raic;
using namespace llvm;
using namespace std;

void PerturbationExecutor::run(AbstractState *as, const ValueRange &v, std::set<llvm::Value*> &VSet){
    for(auto i : p_.getDeltas()){
        //TODO: recover the definition of VSet
//        if(VSet.find(i.first) != VSet.end()){
//            continue;
//        }
        
        double max_perturbation(0); //we only need to calculate the max perturbation;
        DifferentialMap max_perturbation_diff;
        for(auto d : i.second){
            if(ConstantFP *C = dyn_cast<ConstantFP>(d)){
                //max_perturbation += abs(C->getValueAPF().convertToDouble());
                max_perturbation += abs(C->getValueAPF().convertToDouble());
            } else {
                visit(cast<Instruction>(d));
                
                max_perturbation += max(abs(p_range_[d].lower()), abs(p_range_[d].upper()));
                for(auto j: p_diff_[d]){
                    max_perturbation_diff[j.first] += j.second;
                }
            }
        }
        as ->addDiff(ValueRange(max_perturbation), i.first);
        if(VSet.find(i.first) != VSet.end()){
            for(auto j : max_perturbation_diff){
                as ->subgradient_map_[i.first][j.first] = j.second;
            }
       }
    }
}

void PerturbationExecutor::visitFAdd(BinaryOperator &I){
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    
    ValueRange A_vr = getOpValueRange(A);
    ValueRange B_vr = getOpValueRange(B);
    
    p_range_[&I] = A_vr + B_vr;
    p_diff_[&I] = DifferentialMap();
    
    if(!isa<Constant>(A)){
        for(auto i : getOpDiff(A)){
            p_diff_[&I][i.first] += i.second;
        }
    }
    
    if(!isa<Constant>(B)){
        for(auto i : getOpDiff(B)){
            p_diff_[&I][i.first] += i.second;
        }
    }
}

void PerturbationExecutor::visitFSub(BinaryOperator &I){
    
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    
    ValueRange A_vr = getOpValueRange(A);
    ValueRange B_vr = getOpValueRange(B);
    p_range_[&I] = A_vr - B_vr;
    p_diff_[&I] = DifferentialMap();
    
    if(!isa<Constant>(A)){
        for(auto i : getOpDiff(A)){
            p_diff_[&I][i.first] += i.second;
        }
    }
    
    if(!isa<Constant>(B)){
        for(auto i : getOpDiff(B)){
            p_diff_[&I][i.first] -= i.second;
        }
    }

}

void PerturbationExecutor::visitFMul(BinaryOperator &I){
    
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    ValueRange A_vr = getOpValueRange(A);
    ValueRange B_vr = getOpValueRange(B);
    p_range_[&I] = A_vr * B_vr;
    
    p_diff_[&I] = DifferentialMap();
    if(!isa<Constant>(A)){
        for(auto i : getOpDiff(A)){
             p_diff_[&I][i.first] += B_vr * i.second;
        }
    }
    
    if(!isa<Constant>(B)){
        for(auto i : getOpDiff(B)){
            p_diff_[&I][i.first] += A_vr * i.second;
        }
    }
    
}

void PerturbationExecutor::visitFDiv(BinaryOperator &I){
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    ValueRange A_vr = getOpValueRange(A);
    ValueRange B_vr = getOpValueRange(B);
    p_range_[&I] = A_vr / B_vr;
    
    p_diff_[&I] = DifferentialMap();
    if(!isa<Constant>(A)){
        for(auto i : getOpDiff(A)){
            p_diff_[&I][i.first] += B_vr * i.second;
        }
    }
    
    if(!isa<Constant>(B)){
        for(auto i : getOpDiff(B)){
            p_diff_[&I][i.first] -= A_vr * i.second;
        }
    }
    
    for(auto i:p_diff_[&I]){
        p_diff_[&I][i.first] /= (B_vr * B_vr);
    }
}

ValueRange PerturbationExecutor::getOpValueRange(llvm::Value *V){

    ValueRange vr;
    if(value_state_map_.hasAbstractState(V) || isa<Constant>(V)){
        AbstractState *t = value_state_map_.atAbstractState(V);
        vr = t -> getValueRange();
    } else if (auto I = dyn_cast<Instruction>(V)){
        visit(I);
        vr = p_range_[V];
    } else {
        assert(false && "empty abstract state");
    }
    
    return vr;
}

DifferentialMap& PerturbationExecutor::getOpDiff(llvm::Value *V){
    
    if(value_state_map_.hasAbstractState(V)){
         AbstractState *t = value_state_map_.atAbstractState(V);
         return t -> condition_map_;
    }else if(p_diff_.find(V) != p_diff_.end()){
         return p_diff_[V];
    }
    assert(false && "empty abstract state");
}
