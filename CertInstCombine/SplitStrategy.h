//
//  SplitStrategy.h
//  Project
//
//  Created by Gu Yijia on 12/21/17.
//

#ifndef SplitStrategy_h
#define SplitStrategy_h

#include "AbstractState.h"
#include "SplitInfoState.h"
#include "../AnnoInstCombine/Perturbation.h"

#include <vector>
#include <map>

namespace raic {
    class SplitStrategy{
    public:
        SplitStrategy() = default;
        SplitStrategy(std::vector<ValueRange> &input_ranges):input_ranges_(input_ranges){}
        
        std::vector<SplitStrategy> split() const;

        std::vector<ValueRange> getInputsRange() const{
            return input_ranges_;
        }
        
        double getMaxInputsLen() const{
            double maxLen = 0;
            for(auto i : input_ranges_){
                double len = i.upper() - i.lower();
                maxLen = std::max(maxLen, len);
            }
            
            return maxLen;
        }
        
        std::vector<ValueRange> getInputsLowEnd() const{
            std::vector<ValueRange> low;
            for(auto i : input_ranges_){
                low.push_back(i.lower());
            }
            return low;
        }
        
        std::vector<ValueRange> getInputHighEnd() const{
            std::vector<ValueRange> high;
            for(auto i : input_ranges_){
                high.push_back(i.upper());
            }
            return high;
        }
        
        std::vector<ValueRange> getInputMidEnd() const{
            std::vector<ValueRange> mid;
            for(auto i : input_ranges_){
                mid.push_back((i.lower() + i.upper())/2);
            }
            return mid;
        }
        
        
    private:
        std::vector<ValueRange> input_ranges_;
    };
}

#endif /* SplitStrategy_h */
