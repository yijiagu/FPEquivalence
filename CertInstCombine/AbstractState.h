//
//  AbstractState.h
//  Project
//
//  Created by Gu Yijia on 12/22/17.
//

#ifndef AbstractState_h
#define AbstractState_h

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instruction.h"

#include "ErrorInfo.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <limits>

namespace raic{
    /// Class to represent the abstract value
    class AbstractState {
    public:
        void clearState(){
            value_range_ = 0;
            taylor_form.clear();
        }

        AbstractState(){
            
        }
        
        AbstractState(llvm::Type *t, const double c1, const double c2){
            value_type = t;
            value_range_ = ValueRange(c1, c2);
            if(c1 != c2){ //if c1 != c2, there are real numbers in [c1, c2], we need to add a rounding operation as in FPTaylor
                taylor_form[nextTaylorIndex()] = ErrorInfo(value_range_, -53);
            }
        }
        
        AbstractState(llvm::Type *t, const ValueRange &vr){
            value_type = t;
            value_range_ = vr;
        }
        
        void setValueRange(const ValueRange &vr){
            value_range_ = vr;
        }
        /*
         getValueRange() : get the real value range of the abtract state
         getErrorRange() : get the rounding error range of the abstract state
         getTotalRange() : get the total range of the abstract state including value range and error range
         getMaxError() : get the maximum rounding error of the abstract state
         */
        ValueRange getValueRange() const {
            return value_range_;
        }
        ValueRange getErrorRange() const;
        ValueRange getTotalRange() const;
        double getMaxError() const;
        double getMinError() const;
        
        
        llvm::Type *value_type; // the type of the abstract state
        
        ValueRange value_range_; //!< possible range of the value of the node
        //taylor_form[-1]: higher order errors
        std::unordered_map<int, ErrorInfo> taylor_form;
        
        static int next_taylor_index;
        static int nextTaylorIndex(){
            return next_taylor_index ++;
        }
        
        friend llvm::raw_ostream &operator<<(llvm::raw_ostream &out, const AbstractState &as);
    };
    
    AbstractState operator+(AbstractState &a, AbstractState &b);
    AbstractState operator-(AbstractState &a, AbstractState &b);
    AbstractState operator*(AbstractState &a, AbstractState &b);
    AbstractState operator/(AbstractState &a, AbstractState &b);
    
    AbstractState inverse(AbstractState &op);
    
    AbstractState sin(AbstractState &op);
    AbstractState exp(AbstractState &op);
    AbstractState cos(AbstractState &op);
    AbstractState tan(AbstractState &op);
    AbstractState atan2(AbstractState &op1, AbstractState &op2);
    AbstractState asin(AbstractState &op);
    AbstractState log(AbstractState &op);
    AbstractState sqrt(AbstractState &op);

    AbstractState natPow(AbstractState &op1, AbstractState &op2);

    void rounding(AbstractState &op);
    
}
#endif /* AbstractState_h */
