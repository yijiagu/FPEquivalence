//
//  IntervalParser.cpp
//  LLVMCertInstCombine
//
//  Created by Gu Yijia on 10/3/18.
//

#include "CertInstCombine.h"

using namespace raic;
using namespace llvm;

bool IntervalParser::parse(cl::Option &O, StringRef ArgName,
                           const std::string &Arg, DoubleInterval &Val) {
    
    char *End = const_cast<char*>(Arg.c_str());
    
    if(*End++ != '['){
        // Print an error message if unrecognized character!
        return O.error("'" + Arg + "' value invalid for input interval!");
    }
    // Parse double part, leaving 'End' pointing to the first non-number char
    Val.lower_bound = strtod(End, &End);
    
    if(*End++ != ';'){
        return O.error("'" + Arg + "' value invalid for input interval!");
    }
    
    Val.upper_bound = strtod(End, &End);
    
    if(*End++ != ']'){
        return O.error("'" + Arg + "' value invalid for input interval!");
    }
    
    return false;
}
