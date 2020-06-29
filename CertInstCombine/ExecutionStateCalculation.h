//
//  ExecutionStateCalculation.h
//  Project
//
//  Created by Gu Yijia on 12/21/17.
//

#ifndef ExecutionStateCalculation_h
#define ExecutionStateCalculation_h

#include "../AnnoInstCombine/Perturbation.h"
#include "ExecutionStateMap.h"

#define LN2 0.693147180559945309417232121458

namespace raic{
    class ExecutionStateCalculation{
    public:
        static void updateAddExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateSubExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateMulExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateDivExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateSinExecutionState(llvm::Value *I, llvm::Value *op,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateExpExecutionState(llvm::Value *I, llvm::Value *op,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateCosExecutionState(llvm::Value *I, llvm::Value *op,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateTanExecutionState(llvm::Value *I, llvm::Value *op,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateAtan2ExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateAsinExecutionState(llvm::Value *I, llvm::Value *op,
                                              ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        static void updateLogExecutionState(llvm::Value *I, llvm::Value *op,
                                             ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
        
        static void updateSqrtExecutionState(llvm::Value *I, llvm::Value *op,
                                            ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet);
    private:
        static void updateExecutionState (llvm::Value *I, llvm::Value *op1, llvm::Value *op2, ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet,
                                         ValueRange (*operateValueRange)(const ValueRange &op1, const ValueRange &op2),
                                         ValueRange (*operateLogrithmicDiff_left)(const ValueRange &op1, const ValueRange &op2),
                                         ValueRange (*operateLogrithmicDiff_right)(const ValueRange &op1, const ValueRange &op2));
        static void updateExecutionState (llvm::Value *I, llvm::Value *op, ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet,
                                         ValueRange (*operateValueRange)(const ValueRange &op),
                                         ValueRange (*operateLogrithmicDiff)(const ValueRange &op));
        static ValueRange operateAddValueRange(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateSubValueRange(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateMulValueRange(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateDivValueRange(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateSinValueRange(const ValueRange &op);
        static ValueRange operateExpValueRange(const ValueRange &op);
        static ValueRange operateCosValueRange(const ValueRange &op);
        static ValueRange operateTanValueRange(const ValueRange &op);
        static ValueRange operateAtan2ValueRange(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateAsinValueRange(const ValueRange &op);
        static ValueRange operateLogValueRange(const ValueRange &op);
        static ValueRange operateSqrtValueRange(const ValueRange &op);
        
        static ValueRange operateAddLogrithmicDiff(const ValueRange &op1, const ValueRange &op2);

        
        static ValueRange operateSubLogrithmicDiff(const ValueRange &op1, const ValueRange &op2);
        
        static ValueRange operateMulLogrithmicDiff(const ValueRange &op1, const ValueRange &op2);
        
        static ValueRange operateDivLogrithmicDiff(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateSinLogrithmicDiff(const ValueRange &op1);
        static ValueRange operateExpLogrithmicDiff(const ValueRange &op1);
        static ValueRange operateCosLogrithmicDiff(const ValueRange &op1);
        static ValueRange operateTanLogrithmicDiff(const ValueRange &op1);
        static ValueRange operateAtan2LogrithmicDiff_left(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateAtan2LogrithmicDiff_right(const ValueRange &op1, const ValueRange &op2);
        static ValueRange operateAsinLogrithmicDiff(const ValueRange &op1);
        static ValueRange operateLogLogrithmicDiff(const ValueRange &op1);
        static ValueRange operateSqrtLogrithmicDiff(const ValueRange &op1);
    };
    
    inline ValueRange ExecutionStateCalculation::operateAddValueRange(const ValueRange &op1, const ValueRange &op2){
        if(boost::numeric::singleton(op1) && boost::numeric::singleton(op2)){
            return ValueRange(op1.lower() + op2.lower());
        }
        return op1 + op2;
    }
    
    inline ValueRange ExecutionStateCalculation::operateSubValueRange(const ValueRange &op1, const ValueRange &op2){
        if(boost::numeric::singleton(op1) && boost::numeric::singleton(op2)){
            return ValueRange(op1.lower() - op2.lower());
        }
        return op1 - op2;
    }
    
    inline ValueRange ExecutionStateCalculation::operateMulValueRange(const ValueRange &op1, const ValueRange &op2){
        if(boost::numeric::singleton(op1) && boost::numeric::singleton(op2)){
            return ValueRange(op1.lower() * op2.lower());
        }
        return op1 * op2;
    }
    
    inline ValueRange ExecutionStateCalculation::operateDivValueRange(const ValueRange &op1, const ValueRange &op2){
        if(boost::numeric::singleton(op1) && boost::numeric::singleton(op2)){
            return ValueRange(op1.lower() / op2.lower());
        }
        return op1 / op2;
    }
    
    inline ValueRange ExecutionStateCalculation::operateSinValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::sin(op.lower()));
        }
        return sin(op);
    }
    
    inline ValueRange ExecutionStateCalculation::operateExpValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::exp(op.lower()));
        }
        return exp(op);
    }
    
    inline ValueRange ExecutionStateCalculation::operateCosValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::cos(op.lower()));
        }
        return cos(op);
    }
    
    inline ValueRange ExecutionStateCalculation::operateTanValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::tan(op.lower()));
        }
        return tan(op);
    }
    
    inline ValueRange ExecutionStateCalculation::operateAtan2ValueRange(const ValueRange &op1, const ValueRange &op2){
        if(boost::numeric::singleton(op1) && boost::numeric::singleton(op2)){
            const double pi = 3.14159265358979323846;
            if(op2.lower() > 0){
                return std::atan(op1.lower()/op2.lower());
            }else if (op2 < 0 && op1 >= 0) {
                return std::atan(op1.lower()/op2.lower()) + pi;
            } else if (op2 < 0 && op1 < 0){
                return std::atan(op1.lower()/op2.lower()) - pi;
            }
            assert(false && "Nondeterminant Atan2 result");
            return 0;
        }
        if(op2 > 0){
          return atan(op1/op2);
        } else if (op2 < 0 && op1 >= 0) {
            return atan(op1/op2) + boost::numeric::interval_lib::pi<ValueRange>();
        } else if (op2 < 0 && op1 < 0){
            return atan(op1/op2) - boost::numeric::interval_lib::pi<ValueRange>();
        }
        assert(false && "Nondeterminant Atan2 result");
        return 0;
    }
    
    inline ValueRange ExecutionStateCalculation::operateAsinValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::asin(op.lower()));
        }
        return asin(op);
    }
    
    inline ValueRange ExecutionStateCalculation::operateLogValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::log(op.lower()));
        }
        return log(op);
    }
    
    inline ValueRange ExecutionStateCalculation::operateSqrtValueRange(const ValueRange &op){
        if(boost::numeric::singleton(op)){
            return ValueRange(std::sqrt(op.lower()));
        }
        return sqrt(op);
    }

    inline ValueRange ExecutionStateCalculation::operateAddLogrithmicDiff(const ValueRange &op1, const ValueRange &op2){
        return ValueRange(1);
    }
    
    inline ValueRange ExecutionStateCalculation::operateSubLogrithmicDiff(const ValueRange &op1, const ValueRange &op2){
        return op1/(op1 - op2);
    }
    
    inline ValueRange ExecutionStateCalculation::operateMulLogrithmicDiff(const ValueRange &op1, const ValueRange &op2){
        return op2;
    }
    
    inline ValueRange ExecutionStateCalculation::operateDivLogrithmicDiff(const ValueRange &op1, const ValueRange &op2){
        return ValueRange(1);
    }
    
    inline ValueRange ExecutionStateCalculation::operateSinLogrithmicDiff(const ValueRange &op1){
        return op1 * cos(op1) / sin(op1);
    }
    
    inline ValueRange ExecutionStateCalculation::operateExpLogrithmicDiff(const ValueRange &op1){
        return op1;
    }
    
    inline ValueRange ExecutionStateCalculation::operateCosLogrithmicDiff(const ValueRange &op1){
        return -op1 * sin(op1) / cos(op1);
    }
    
    inline ValueRange ExecutionStateCalculation::operateTanLogrithmicDiff(const ValueRange &op1){
        return op1/(cos(op1) * cos(op1) * tan(op1));
    }
    
    inline ValueRange ExecutionStateCalculation::operateAtan2LogrithmicDiff_left(const ValueRange &op1, const ValueRange &op2){
        return -op1*op1/((op1*op1 + op2*op2) * atan(op1/op2));
    }
    
    inline ValueRange ExecutionStateCalculation::operateAtan2LogrithmicDiff_right(const ValueRange &op1, const ValueRange &op2){
        return op2*op2/((op1*op1 + op2*op2) * atan(op1/op2));
    }
    
    inline ValueRange ExecutionStateCalculation::operateAsinLogrithmicDiff(const ValueRange &op1){
        return op1/(sqrt(ValueRange(1) - op1) * asin(op1));
    }
    
    inline ValueRange ExecutionStateCalculation::operateLogLogrithmicDiff(const ValueRange &op1){
        return ValueRange(1)/log(op1)*ValueRange(LN2);
    }
    
    inline ValueRange ExecutionStateCalculation::operateSqrtLogrithmicDiff(const ValueRange &op1){
        return ValueRange(1/2);
    }
    
    inline void ExecutionStateCalculation::updateAddExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                                                    ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op1, op2, value_state_map, perturbation_map, VSet, &operateAddValueRange, &operateAddLogrithmicDiff, &operateAddLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateSubExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                                                    ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        
        updateExecutionState(I, op1, op2, value_state_map, perturbation_map, VSet, &operateSubValueRange, &operateSubLogrithmicDiff, &operateSubLogrithmicDiff);
        
    }
    
    inline void ExecutionStateCalculation::updateMulExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                                                    ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op1, op2, value_state_map, perturbation_map, VSet, &operateMulValueRange, &operateMulLogrithmicDiff, &operateMulLogrithmicDiff);
        
    }
    
    inline void ExecutionStateCalculation::updateDivExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                                                    ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op1, op2, value_state_map, perturbation_map, VSet, &operateDivValueRange, &operateDivLogrithmicDiff, &operateDivLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateSinExecutionState(llvm::Value *I, llvm::Value *op,
                                        ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateSinValueRange, &operateSinLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateExpExecutionState(llvm::Value *I, llvm::Value *op,
                                                                   ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateExpValueRange, &operateExpLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateCosExecutionState(llvm::Value *I, llvm::Value *op,
                                        ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateCosValueRange, &operateCosLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateTanExecutionState(llvm::Value *I, llvm::Value *op,
                                        ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateTanValueRange, &operateTanLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateAtan2ExecutionState(llvm::Value *I, llvm::Value *op1, llvm::Value *op2,
                                                                   ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op1, op2, value_state_map, perturbation_map, VSet, &operateAtan2ValueRange, &operateAtan2LogrithmicDiff_left, &operateAtan2LogrithmicDiff_right);
    }
    
    inline void ExecutionStateCalculation::updateAsinExecutionState(llvm::Value *I, llvm::Value *op,
                                                                   ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateAsinValueRange, &operateAsinLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateLogExecutionState(llvm::Value *I, llvm::Value *op,
                                                                    ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateLogValueRange, &operateLogLogrithmicDiff);
    }
    
    inline void ExecutionStateCalculation::updateSqrtExecutionState(llvm::Value *I, llvm::Value *op,
                                                                   ExecutionStateMap &value_state_map,  llvm::PerturbationMap &perturbation_map, std::set<llvm::Value*> &VSet){
        updateExecutionState(I, op, value_state_map, perturbation_map, VSet, &operateLogValueRange, &operateLogLogrithmicDiff);
    }
}
#endif /* ExecutionStateCalculation_h */
