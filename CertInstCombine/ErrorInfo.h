//
//  ErrorInfo.h
//  Project
//
//  Created by Gu Yijia on 7/29/19.
//

#ifndef ErrorInfo_h
#define ErrorInfo_h

#include <unordered_map>
#include <boost/numeric/interval.hpp>

namespace raic {
    using ValueRange = boost::numeric::interval<double,
    boost::numeric::interval_lib::policies<boost::numeric::interval_lib::save_state_nothing<boost::numeric::interval_lib::rounded_transc_exact<double> >,
    boost::numeric::interval_lib::checking_base<double> > >;
    
    struct ErrorInfo{
        ValueRange err_range = 0;
        int exp = 0; //The upper bound of the error is 2^exp, exp = 0 means that the ErrorInfo is empty
        std::unordered_map<int, ValueRange> wilkinson_coefficients; //record the set of wilkinson numbers
        
        ErrorInfo():err_range(0), exp(0){}
        ErrorInfo(const ValueRange &v, const int e):err_range(v), exp(e){}
        ErrorInfo(const ValueRange &v, const int e, const std::unordered_map<int, ValueRange> &coeff):err_range(v), exp(e), wilkinson_coefficients(coeff){}
        
        
        ErrorInfo multValueRange(const ValueRange &vr){
            return ErrorInfo(err_range * vr, exp, wilkinson_coefficients);
        }
        
        ErrorInfo getAbsHigh() const{
            return ErrorInfo(fmax(abs(err_range.upper()), abs(err_range.lower())), exp, wilkinson_coefficients);
        }
        
        ErrorInfo getAbsLow() const{
            return ErrorInfo(fmin(abs(err_range.upper()), abs(err_range.lower())), exp, wilkinson_coefficients);
        }
        
        ErrorInfo getNegation() const{
            return ErrorInfo(-err_range, exp, wilkinson_coefficients);
        }
        
        
    };
    
    ErrorInfo operator+(const ErrorInfo &a, const ErrorInfo &b);
    ErrorInfo operator-(const ErrorInfo &a, const ErrorInfo &b);
    ErrorInfo operator*(const ErrorInfo &a, const ErrorInfo &b);
}


#endif /* ErrorInfo_h */
