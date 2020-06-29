//
//  SplitInfoState.h
//  Project
//
//  Created by Gu Yijia on 1/2/18.
//

#ifndef SplitInfoState_h
#define SplitInfoState_h

#include "AbstractState.h"

namespace raic {
    using DiffArgMap = std::map<llvm::Value *, std::vector<ValueRange>>;
    
    class SplitInfoState{
    public:
        SplitInfoState() = default;
        SplitInfoState(std::vector<ValueRange> &input_ranges):input_ranges_(input_ranges){}
        bool splitable() const;
        std::vector<SplitInfoState> split();
        void updateSplitDimension(const DiffArgMap &diff_argument_map);
        
        const ValueRange &getInputRange(int i) const{
            return input_ranges_[i];
        }
        
        
    private:
        double termination_epsilon = 0.00001; //TODO: set reasonable termination epsilon
        int split_dim_ = 0;
        std::vector<ValueRange> input_ranges_;
        //AbstractState target_as_; //represent the abstract state of the return value
        
        friend bool operator<(const SplitInfoState& lhs, const SplitInfoState& rhs);
    };
}
#endif /* SplitInfoState_h */
