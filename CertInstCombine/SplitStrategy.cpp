//
//  SplitStrategy.cpp
//  LLVMAnnoInstCombine
//
//  Created by Gu Yijia on 3/12/18.
//

#include "SplitStrategy.h"

#include "llvm/Support/Format.h"

using namespace raic;
using namespace llvm;


std::vector<SplitStrategy> SplitStrategy::split() const{
    std::vector<SplitStrategy> sv;
    SplitStrategy s1, s2;
    int split_dim = -1;
    double maxLen = 0;
    for(int i = 0; i < input_ranges_.size(); i ++){
       double len = input_ranges_[i].upper() - input_ranges_[i].lower();
        if(len > maxLen){
            maxLen = len;
            split_dim = i;
        }
    }
    
    s1.input_ranges_ = input_ranges_;
    s2.input_ranges_ = input_ranges_;
    s1.input_ranges_[split_dim] = ValueRange(input_ranges_[split_dim].lower(), boost::numeric::median(input_ranges_[split_dim]));
    s2.input_ranges_[split_dim] = ValueRange(boost::numeric::median(input_ranges_[split_dim]), input_ranges_[split_dim].upper());
    sv.push_back(s1);
    sv.push_back(s2);
    return sv;
}

