//
//  SplitInfoState.cpp
//  LLVMAnnoInstCombine
//
//  Created by Gu Yijia on 1/2/18.
//

#include "SplitInfoState.h"

#include "llvm/IR/Function.h"

using namespace raic;
using namespace llvm;

bool SplitInfoState::splitable() const{
    if(boost::numeric::width(input_ranges_.at(split_dim_)) <= termination_epsilon){
        return false;
    }
    return true;
}

std::vector<SplitInfoState> SplitInfoState::split(){
    std::vector<SplitInfoState> sv;
    SplitInfoState s1, s2;
    s1.input_ranges_ = input_ranges_;
    s2.input_ranges_ = input_ranges_;
    s1.input_ranges_[split_dim_] = ValueRange(input_ranges_[split_dim_].lower(), boost::numeric::median(input_ranges_[split_dim_]));
    s2.input_ranges_[split_dim_] = ValueRange(boost::numeric::median(input_ranges_[split_dim_]), input_ranges_[split_dim_].upper());
    sv.push_back(s1);
    sv.push_back(s2);
    return sv;
}

void SplitInfoState::updateSplitDimension(const DiffArgMap &diff_argument_map){
    //TODO::
    split_dim_ = 0;
}

bool raic::operator<(const SplitInfoState& lhs, const SplitInfoState& rhs){
    return true;//lhs.target_as_.getPerturbationUpperBound().second.upper() < rhs.target_as_.getPerturbationUpperBound().second.upper();
}
