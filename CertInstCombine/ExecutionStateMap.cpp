//
//  ExecutionStateMap.cpp
//  LLVMCertInstCombine
//
//  Created by Gu Yijia on 12/22/17.
//

#include "ExecutionStateMap.h"

using namespace raic;
using namespace llvm;
using namespace std;

bool ExecutionStateMap::isTrueConstant(llvm::Value *V) const{
    ConstantInt *C;
    if(isa<ConstantInt>(V)){
        C = cast<ConstantInt>(V);
        return C == C->getTrue(V->getType()->getContext());
    }else if(value_state_map_.find(V) != value_state_map_.end()){
        const AbstractState *as = value_state_map_.at(V).get();
        if((as -> getValueRange().upper() == as -> getValueRange().lower()) && as -> getValueRange().lower() != 0){
            return true;
        }
    }
    return false;
}

bool ExecutionStateMap::isFalseConstant(llvm::Value *V) const{
    ConstantInt *C;
    if(isa<ConstantInt>(V)){
        C = cast<ConstantInt>(V);
        return C == C->getFalse(V->getType()->getContext());
    }else if(value_state_map_.find(V) != value_state_map_.end()){
        const AbstractState *as = value_state_map_.at(V).get();
        if(as -> getValueRange().upper() == 0 && as -> getValueRange().lower() == 0){
            return true;
        }
    }
    return false;
}

AbstractState *ExecutionStateMap::atAbstractState(Value *V) {
    if(value_state_map_.find(V)  == value_state_map_.end()){ //current ExecutionStateMap does not contain V
        if(ConstantFP *C = dyn_cast<ConstantFP>(V)){ // V is a ConstantFP
         
            double c;
            if(&(C -> getValueAPF().getSemantics()) == &(APFloat::IEEEsingle()))
                c = C->getValueAPF().convertToFloat();
            else
                c = C->getValueAPF().convertToDouble();
            
            AbstractState *s = new AbstractState(V -> getType(), c, c);
            value_state_map_[V] = unique_ptr<AbstractState>(s);
        }else if(ConstantInt *C = dyn_cast<ConstantInt>(V)){
            int c = C -> getSExtValue();
            AbstractState *s = new AbstractState(V -> getType(), c, c);
            value_state_map_[V] = unique_ptr<AbstractState>(s);
        }else { // generate empty AbstractState
            //todo : report error
            value_state_map_[V] = unique_ptr<AbstractState>(new AbstractState(V -> getType(), 0, 0));
        }
    }
    
    return value_state_map_[V].get();
}



void ExecutionStateMap::copyValue(llvm::Value *S, llvm::Value *D){
    Type* dTy = D -> getType();
    Type* sTy = S -> getType();
    if((dTy -> isPointerTy() && dTy->getPointerElementType() -> isPointerTy()) ||
       (sTy -> isPointerTy() && sTy -> getPointerElementType() -> isPointerTy())){ // D or S is a p**
        assert(pointer_map_.find(S) != pointer_map_.end() && "Copy Null Pointer");
        pointer_map_[D] = pointer_map_[S];
    } else {
        if(pointer_map_.find(S) != pointer_map_.end()){ //S is a pointer to aggregate structure
            PointerIndex &pi = pointer_map_[S];
            auto t = aggregate_state_map_[pi.base][pi.index].get();
            
            if(pointer_map_.find(D) != pointer_map_.end()){ //D is a pointer to agregate structure
                PointerIndex &pi_d = pointer_map_[D];
                aggregate_state_map_[pi_d.base][pi_d.index] = std::unique_ptr<AbstractState>(new AbstractState(*t));
            } else { // D is a single value
                value_state_map_[D] = std::unique_ptr<AbstractState>(new AbstractState(*t));
            }
        }
        else {
            auto t = atAbstractState(S);
            if(pointer_map_.find(D) != pointer_map_.end()){ //D is a pointer to aggregate structure
                PointerIndex &pi_d = pointer_map_[D];
                aggregate_state_map_[pi_d.base][pi_d.index] = std::unique_ptr<AbstractState>(new AbstractState(*t));
            } else { // D is a single value
                value_state_map_[D] = std::unique_ptr<AbstractState>(new AbstractState(*t));
            }
        }
    }
}

std::vector<std::unique_ptr<AbstractState>>& ExecutionStateMap::atAggregateState(llvm::Value *V){
    if(aggregate_state_map_.find(V) == aggregate_state_map_.end()){
        aggregate_state_map_[V] = std::vector<std::unique_ptr<AbstractState>>();
        Type *ty = V -> getType();
        if(ty -> isVectorTy()){
            if(Constant *CV = dyn_cast<Constant>(V)){
                for(int i = 0; i < ty -> getVectorNumElements(); i ++){
                    if(ConstantFP *C = dyn_cast<ConstantFP>(CV -> getAggregateElement(i))){ // V is a ConstantFP
                        
                        double c;
                        if(&(C -> getValueAPF().getSemantics()) == &(APFloat::IEEEsingle()))
                            c = C->getValueAPF().convertToFloat();
                        else
                            c = C->getValueAPF().convertToDouble();
                        AbstractState *s = new AbstractState(V -> getType(), c, c);
                        aggregate_state_map_[V].push_back(unique_ptr<AbstractState>(s));
                    }else if(ConstantInt *C = dyn_cast<ConstantInt>(CV -> getAggregateElement(i))){
                        int c = C -> getSExtValue();
                        AbstractState *s = new AbstractState(V -> getType(), c, c);
                        aggregate_state_map_[V].push_back(unique_ptr<AbstractState>(s));
                    }else{ //undef value
                        aggregate_state_map_[V].push_back(make_unique<AbstractState>(V -> getType(), 0, 0));
                    }
                }
            }
        }
        
    }
    return aggregate_state_map_[V];
}

bool ExecutionStateMap::hasPointerIndex(llvm::Value *V, PointerIndex &pi) {
    if(pointer_map_.find(V) != pointer_map_.end()){
        pi = pointer_map_[V];
        return true;
    }else if(aggregate_state_map_.find(V) != aggregate_state_map_.end()){
        pi.base = V;
        pi.index = 0;
        return true;
    }
    return false;
}
