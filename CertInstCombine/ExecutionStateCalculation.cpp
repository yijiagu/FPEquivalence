//
//  ExecutionStateCalculation.cpp
//  LLVMCertInstCombine
//
//  Created by Gu Yijia on 12/21/17.
//

#include "ExecutionStateCalculation.h"
#include "PerturbationExecutor.h"

#include <boost/numeric/interval.hpp>

using namespace raic;
using namespace llvm;

void ExecutionStateCalculation::updateExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                                      ExecutionStateMap &value_state_map,
                                                      llvm::PerturbationMap &perturbation_map,
                                                      std::set<llvm::Value*> &VSet,
                                                      ValueRange (*operateValueRange)(const ValueRange &op1, const ValueRange &op2),
                                                      ValueRange (*operateLogrithmicDiff_left)(const ValueRange &op1, const ValueRange &op2),
                                                      ValueRange (*operateLogrithmicDiff_right)(const ValueRange &op1, const ValueRange &op2)){
    AbstractState *abstract_A = value_state_map.atAbstractState(op1);
    AbstractState *abstract_B = value_state_map.atAbstractState(op2);
    AbstractState *abstract_I = value_state_map.atAbstractState(I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    //update ValueRange
    ValueRange v = operateValueRange(abstract_A -> getValueRange(), abstract_B -> getValueRange());
    abstract_I -> setValueRange(v);


    
    
    //Calculate logarithmic differential with respect to new delta introduced in current I
    if(perturbation_map.find(I) != perturbation_map.end()){ //Instruction contains newly introduced delta
        DEBUG(dbgs() << "Perturnbation Name: " << I << "\n");
        //Calculate logarithmi differential with respect to delta
        Perturbation &p = perturbation_map[I];
        PerturbationExecutor pe(p, value_state_map);
        pe.run(abstract_I, v, VSet);
        //ValueRange abs_delta = pe.run();
        //ValueRange abs_v = v.lower() > 0? v : ValueRange(-v.upper(), -v.lower());
        //abstract_I ->addDiff(abs_delta/v, I);
        DEBUG(dbgs() << "Calculate New perturbation: " << *abstract_I << "\n");
    }
}

void ExecutionStateCalculation::updateExecutionState(llvm::Value *I, llvm::Value *op,
                                                     ExecutionStateMap &value_state_map,
                                                     llvm::PerturbationMap &perturbation_map,
                                                     std::set<llvm::Value*> &VSet,
                                                     ValueRange (*operateValueRange)(const ValueRange &op),
                                                     ValueRange (*operateLogrithmicDiff)(const ValueRange &op)){
    AbstractState *abstract_A = value_state_map.atAbstractState(op);
    AbstractState *abstract_I = value_state_map.atAbstractState(I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    //update ValueRange
    ValueRange v = operateValueRange(abstract_A -> getValueRange());
    abstract_I -> setValueRange(v);
    


    //Calculate logarithmic differential with respect to new delta introduced in current I
    if(perturbation_map.find(I) != perturbation_map.end()){ //Instruction contains newly introduced delta
        DEBUG(dbgs() << "Perturnbation Name: " << I << "\n");
        //Calculate logarithmi differential with respect to delta
        Perturbation &p = perturbation_map[I];
        PerturbationExecutor pe(p, value_state_map);
        //ValueRange abs_delta = pe.run();
        pe.run(abstract_I, v, VSet);
        //ValueRange abs_v = v.lower() > 0? v : ValueRange(-v.upper(), -v.lower());
        //abstract_I ->addDiff(abs_delta/v, I);
        DEBUG(dbgs() << "Calculate New perturbation: " << *abstract_I << "\n");
    }
}


