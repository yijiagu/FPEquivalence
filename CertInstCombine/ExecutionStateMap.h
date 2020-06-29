//
//  ExecutionStateMap.h
//  Project
//
//  Created by Gu Yijia on 12/22/17.
//

#ifndef ExecutionStateMap_h
#define ExecutionStateMap_h

#include "AbstractState.h"
#include "PointerIndex.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/Constants.h"

#include <map>

namespace raic {
    
    class ExecutionStateMap {
    public:
        void update(llvm::Value *V, std::unique_ptr<AbstractState> &as);
        void update(llvm::Value *V, int index, std::unique_ptr<AbstractState> &as);
        void update(llvm::Value *V, PointerIndex &pi);
        void update(llvm::Value *V, llvm::Constant *C);
        
        void copyValue(llvm::Value *S, llvm::Value *D);
        
        AbstractState *atAbstractState(llvm::Value *);
        std::vector<std::unique_ptr<AbstractState>>& atAggregateState(llvm::Value *V);
        
        bool hasPointerIndex(llvm::Value *V, PointerIndex &pi);
        
        bool hasAbstractState(llvm::Value *V){
            return value_state_map_.find(V) != value_state_map_.end();
        }
        
        bool isTrueConstant(llvm::Value *V) const;
        bool isFalseConstant(llvm::Value *V) const;
        
    private:
        std::map<llvm::Value*, std::unique_ptr<AbstractState>> value_state_map_; //!< Map values to abstract values
        std::map<llvm::Value*, std::vector<std::unique_ptr<AbstractState> >> aggregate_state_map_;
        std::map<llvm::Value*, PointerIndex> pointer_map_;
    };
    
    inline void ExecutionStateMap::update(llvm::Value *k, llvm::Constant *v){
        AbstractState *as = nullptr;

        if(llvm::ConstantFP *C= llvm::dyn_cast<llvm::ConstantFP>(v)){
            double c;
            if(&(C -> getValueAPF().getSemantics()) == &(llvm::APFloat::IEEEsingle()))
                c = C->getValueAPF().convertToFloat();
            else
                c = C->getValueAPF().convertToDouble();
            as = new AbstractState(k -> getType(), c, c);
        }else if(llvm::ConstantInt *C = llvm::dyn_cast<llvm::ConstantInt>(v)){
            int c = C -> getSExtValue();
            as = new AbstractState(k -> getType(), c, c);
        }else{
            assert (false && "unsupported Constant type");
        }
       
        value_state_map_[k] = std::unique_ptr<AbstractState>(as);
    }
    
    inline void ExecutionStateMap::update(llvm::Value *k, std::unique_ptr<AbstractState> &as){
        value_state_map_[k] = std::move(as);
    }
    
    inline void ExecutionStateMap::update(llvm::Value *k, PointerIndex &pi){
        pointer_map_[k] = pi;
    }
    
    inline void ExecutionStateMap::update(llvm::Value *k, int index, std::unique_ptr<AbstractState> &as){
       aggregate_state_map_[k][index] = std::move(as);
    }
}

#endif /* ExecutionStateMap_h */
