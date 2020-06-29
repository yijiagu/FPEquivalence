//
//  AbstractState.cpp
//  LLVMCertInstCombine
//
//  Created by Gu Yijia on 12/22/17.
//
#include <iostream>
#include "AbstractState.h"
#include "llvm/Support/Format.h"

#define LN2 0.693147180559945309417232121458

/*
 TODO: check fadd, fminus do not need to consider subnormal
 */

using namespace raic;
using namespace llvm;
using namespace std;

int AbstractState::next_taylor_index = 0;

raw_ostream &raic::operator<<(raw_ostream &out, const AbstractState &as){
    out << "Value Range ["
    <<  format("%.17f",as.value_range_.lower()) << "," << format("%.17f",as.value_range_.upper())
    << "]\n";
    out << "Delta Info" << "\n";
    double maxAbs = 0;
    out << "Total Perturbation" << format("%.17f", maxAbs) << "\n";
#ifdef SINGLE_FP_TYPE
    ValueRange t = as.value_range_ * maxAbs * (double)numeric_limits<float>::epsilon();
    out << "ULP" << format("%.17f", as.value_range_.lower() * (double)numeric_limits<float>::epsilon()) << "\n";
#else
    ValueRange t = as.value_range_ * maxAbs * numeric_limits<double>::epsilon();
    out << "ULP" << format("%.17f", as.value_range_.lower() * numeric_limits<double>::epsilon()) << "\n";
#endif
    
    out << " Absolute Perturbation: [" << format("%.17f", t.lower()) << "," << format("%.17f", t.upper())
    << "]\n";
    return out;
}
    
ErrorInfo raic::operator+(const ErrorInfo &a, const ErrorInfo &b){
    if(a.exp < b.exp){
        return b + a;
    }
    
    if(a.exp == 0){
        if(b.exp == 0){
            return ErrorInfo();
        }
        return b;
    }else if(b.exp == 0){
        return a;
    }else if(a.exp == b.exp){
        return ErrorInfo(a.err_range + b.err_range, a.exp);
    }else{
        double eps = ldexp(1.0, a.exp - b.exp);
        return ErrorInfo(a.err_range * eps + b.err_range, b.exp);
    }
}

ErrorInfo raic::operator-(const ErrorInfo &a, const ErrorInfo &b){
    ErrorInfo neg(-b.err_range, b.exp);
    return a + neg;
}

ErrorInfo raic::operator*(const ErrorInfo &a, const ErrorInfo &b){
    return ErrorInfo(a.err_range * b.err_range, a.exp + b.exp);
}

double AbstractState::getMaxError() const {
    ErrorInfo e_info;
    for(auto i : taylor_form){
        e_info = e_info + i.second.getAbsHigh();
    }
    return ldexp(1.0, e_info.exp) * e_info.err_range.upper();
}
    
double AbstractState::getMinError() const {
    ErrorInfo e_info;
    for(auto i : taylor_form){
        e_info = e_info + i.second.getAbsLow();
    }
    return ldexp(1.0, e_info.exp) * e_info.err_range.lower();
}
    
ValueRange AbstractState::getErrorRange() const {
    double max_error = getMaxError();
    return ValueRange(-max_error, max_error);
}

ValueRange AbstractState::getTotalRange() const {
        return getValueRange() + getErrorRange();
}

void raic::rounding(AbstractState &op){
    ValueRange s1 = op.getErrorRange();
    op.taylor_form[-1] = op.taylor_form[-1] + ErrorInfo(s1, -53);
    op.taylor_form[AbstractState::nextTaylorIndex()] = ErrorInfo(op.value_range_, -53);
}

AbstractState raic::operator+(AbstractState &a, AbstractState &b){
    AbstractState r;
    //set value range;
    r.setValueRange(a.getValueRange() + b.getValueRange());
    r.taylor_form = a.taylor_form;
    
    for(auto iter : b.taylor_form){
    r.taylor_form[iter.first] = r.taylor_form[iter.first] + iter.second;
    }
    
    rounding(r);
    
    return r;
}

AbstractState raic::operator-(AbstractState &a, AbstractState &b){
    AbstractState r;
    //set value range;
    r.setValueRange(a.getValueRange() - b.getValueRange());
    
    r.taylor_form = a.taylor_form;
    
    for(auto iter : b.taylor_form){
    r.taylor_form[iter.first] = r.taylor_form[iter.first] - iter.second;
    }

    rounding(r);
    return r;
}


AbstractState raic::operator*(AbstractState &a, AbstractState &b){
    AbstractState r;
    //set value range;
    r.setValueRange(a.getValueRange() * b.getValueRange());
    
    for(auto iter : a.taylor_form){
    r.taylor_form[iter.first] = iter.second.multValueRange(b.getValueRange());
    }
    
    for(auto iter : b.taylor_form){
    r.taylor_form[iter.first] = r.taylor_form[iter.first] + iter.second.multValueRange(a.getValueRange());
    }

    ValueRange error_mult = a.getErrorRange() * b.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(error_mult, -53);
    
    rounding(r);
    
    return r;
}

AbstractState raic::operator/(AbstractState &a, AbstractState &b){
    AbstractState r;
    r = inverse(b);
    r = a * r;
    return r;
}
    
AbstractState raic::inverse(AbstractState &op){
    AbstractState r;
    r.setValueRange(1.0 / op.getValueRange());
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = tr * tr * tr;
    if(d.lower() <= 0 && d.upper() >= 0){
    assert(false && "Division by zero");
    }
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    
    auto b_high = abs(1.0 / d).upper();
    auto m3 = b_high * error_mult;
    
    ValueRange m1 = -1.0/(op.getValueRange() * op.getValueRange());
    for(auto iter : op.taylor_form){
    r.taylor_form[iter.first] = r.taylor_form[iter.first] + iter.second.multValueRange(m1);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);


    return r;
}

AbstractState raic::exp(AbstractState &op){
    AbstractState r;
    
    //set value range;
    r.setValueRange(exp(op.getValueRange()));
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = exp(tr);
    auto b_high = (0.5 * abs(d)).upper();
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    
    auto m3 = b_high * error_mult;
    
    for(auto i : op.taylor_form){
        r.taylor_form[i.first] = ErrorInfo(exp(op.getValueRange()) * i.second.err_range, i.second.exp);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
    rounding(r);
    
    return r;
}
    
AbstractState raic::cos(AbstractState &op){
    AbstractState r;
        
    //set value range;
    r.setValueRange(cos(op.getValueRange()));
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = cos(tr);
    auto b_high = (0.5 * abs(d)).upper();
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult/eps;
    
    auto m3 = b_high * error_mult;
    
    for(auto i:op.taylor_form){
       r.taylor_form[i.first] = ErrorInfo(sin(op.getValueRange()) * i.second.err_range, i.second.exp);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
    rounding(r);
        
    return r;
}
    
AbstractState raic::sin(AbstractState &op){
    AbstractState r;
    
    //set value range:
    r.setValueRange(sin(op.getValueRange()));
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = sin(tr);
    auto b_high = (0.5 * abs(d)).upper();
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult/eps;
    
    auto m3 = b_high * error_mult;
    
    for(auto i : op.taylor_form){
       r.taylor_form[i.first] = ErrorInfo(cos(op.getValueRange()) * i.second.err_range, i.second.exp);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
    rounding(r);
    return r;
}
    
    
AbstractState raic::natPow(AbstractState &op1, AbstractState &op2){
    ValueRange op2Vr = op2.getValueRange();
    
    assert(op2Vr.lower() == op2Vr.upper() && "Ambiguity nat power");

    int exponent = (int)op2Vr.upper();
    assert(exponent >= 3 && "the exponent of pow must >= 3");

    AbstractState r;
    r.setValueRange(pow(op1.getValueRange(), exponent));

    ValueRange tr = op1.getTotalRange();

    ValueRange d = double(exponent * (exponent - 1)) * pow(tr, exponent - 2);

    auto b_high = (0.5 * abs(d)).upper();
    ValueRange error_mult = op1.getErrorRange() * op1.getErrorRange();

    auto eps = ldexp(1.0, -53);
    error_mult = error_mult/eps;

    auto m3 = b_high * error_mult;

    for(auto i : op1.taylor_form){
        r.taylor_form[i.first] = ErrorInfo(double(exponent) * pow(tr, exponent - 1) * i.second.err_range, i.second.exp);
    }

   r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
   rounding(r);
   return r;
}
    
AbstractState raic::log(AbstractState &op){
    AbstractState r;
    
    //set value range;
    r.setValueRange(log(op.getValueRange()));
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = 1.0 / (tr * tr);
    auto b_high = (0.5 * abs(d)).upper();
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    
    auto m3 = b_high * error_mult;
    
    for(auto i : op.taylor_form){
        r.taylor_form[i.first] = ErrorInfo((1.0 / op.getValueRange()) * i.second.err_range, i.second.exp);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
    rounding(r);
    
    
    return r;
}

//implement sqrt
AbstractState raic::sqrt(AbstractState &op){
    AbstractState r;
    
    //set value range;
    r.setValueRange(sqrt(op.getValueRange()));
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = sqrt(tr * tr * tr);
    auto b_high = (0.125 * abs(d)).upper();
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    
    auto m3 = b_high * error_mult;
    
    for(auto i : op.taylor_form){
        r.taylor_form[i.first] = ErrorInfo(i.second.err_range/(2.0 * sqrt(op.getValueRange())), i.second.exp);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
    rounding(r);
    
    return r;
   
}

AbstractState raic::tan(AbstractState &op){
    AbstractState r;
    r.setValueRange(tan(op.getValueRange()));
    
    ValueRange tr = op.getTotalRange();
    ValueRange d = tan(tr) / (cos(tr) * cos(tr));
    auto b_high = (abs(d)).upper();
    
    ValueRange error_mult = op.getErrorRange() * op.getErrorRange();
    auto eps = ldexp(1.0, -53);
    error_mult = error_mult / eps;
    
    auto m3 = b_high * error_mult;
    
    for(auto i : op.taylor_form){
        r.taylor_form[i.first] = ErrorInfo(i.second.err_range/(cos(op.getValueRange()) * cos(op.getValueRange())), i.second.exp);
    }
    
    r.taylor_form[-1] = r.taylor_form[-1] + ErrorInfo(m3, -53);
    rounding(r);
    
    return r;
}


//TODO: implement atan2
AbstractState raic::atan2(AbstractState &y, AbstractState &x){
    AbstractState r;
    //set value range;
    ValueRange atanTwo;
    if(x.getValueRange() > 0){
        atanTwo = atan(y.getValueRange()/x.getValueRange());
    } else if (x.getValueRange() < 0 && y.getValueRange() >= 0) {
        atanTwo = atan(y.getValueRange()/x.getValueRange()) + boost::numeric::interval_lib::pi<ValueRange>();
    } else if (x.getValueRange() < 0 && y.getValueRange() < 0){
        atanTwo = atan(y.getValueRange()/x.getValueRange()) - boost::numeric::interval_lib::pi<ValueRange>();
    } else {
        assert(false && "Nondeterminant Atan2 result");
    }
    
    r.setValueRange(atanTwo);

    ValueRange y_square = y.getValueRange() * y.getValueRange();
    ValueRange x_square = x.getValueRange() * x.getValueRange();
    
    
    return r;
}

//TODO: implement asin
AbstractState raic::asin(AbstractState &op){
    
    AbstractState r;
    
    //set value range;
    r.setValueRange(std::asin(op.getValueRange().lower()));
    
    ValueRange one_minus_x_square = ValueRange(1) - (op.getValueRange() * op.getValueRange());
    

    
    return r;
}




