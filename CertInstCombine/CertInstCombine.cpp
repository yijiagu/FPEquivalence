//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "CertInstCombine.h"
#include "ExecutionStateMap.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"

using namespace std;
using namespace llvm;
using namespace raic;

cl::list<DoubleInterval, bool, IntervalParser> InitialConstraints("initialInv", cl::CommaSeparated,
                                                                  cl::desc("Specify initial interval constraints for each function arguments "));
cl::opt<double> SplitLen("splitLen", cl::desc("Specify the initial split length for input intervals"));

#define DEBUG_TYPE "certinstcombine"

bool CertInstCombine::runOnFunction(Function &F){
    ValueRange stableWN(0);
    int counter = 0;
    double lower_bound = 0.0;
    double upper_bound = 0.0;
    vector<ValueRange> input_ranges;
    for(int i = 0; i < F.arg_size(); i ++){
        input_ranges.push_back(ValueRange(InitialConstraints[i].lower_bound, InitialConstraints[i].upper_bound));
    }
   
    ss_queue_.push(SplitStrategy(input_ranges));
    
    while(!ss_queue_.empty()){
        SplitStrategy ss(ss_queue_.front());
        ss_queue_.pop();
       
        ValueRange vr = getErrorBound(ss.getInputsRange(), F);
        
        DEBUG(dbgs() << ss_queue_.size() << "\n");
        DEBUG(dbgs() << "the lower bound of the difference is: " << vr.lower() << "\n");
        DEBUG(dbgs() << "the upper bound of the difference is: " << vr.upper() << "\n");
   
        ValueRange d_min = getErrorBound(ss.getInputsLowEnd(), F);
        ValueRange d_max = getErrorBound(ss.getInputHighEnd(), F);
        ValueRange d_mid = getErrorBound(ss.getInputMidEnd(), F);
            
        double v2 = max(max(d_min.lower(), d_max.lower()), d_mid.lower());
        lower_bound = max(lower_bound, v2);
            
        if(fabs(vr.upper() - v2) <= f_rel_tol * fabs(v2) + f_abs_tol ||
           ss.getMaxInputsLen() <= x_tol
           //counter >= max_iters
         ){
           DEBUG(dbgs() << fabs(vr.upper() - v2) << "\n");
           DEBUG(dbgs() << f_rel_tol * fabs(v2) + f_abs_tol << "\n");
           DEBUG(dbgs() << ss.getMaxInputsLen() << " " << x_tol << "\n");
                
           upper_bound = max(upper_bound, vr.upper());
           continue;
        }else{
           counter ++;
           vector<SplitStrategy> ss_children = ss.split();
           for(auto i : ss_children){
              ss_queue_.push(i);
           }
        }
    }

    dbgs() << "the max error is: " << upper_bound << "\n";
    dbgs() << "iterations: " << counter << "\n";
    
    return false;
}

ValueRange CertInstCombine::getErrorBound(const std::vector<ValueRange> &input_ranges, llvm::Function &F){
    ExecutionStateMap stateMap;
      int i = 0;
      for (Function::arg_iterator ai = F.arg_begin(), ai_e = F.arg_end(); ai != ai_e; ++ai){
           if(ai -> getType() -> isPointerTy()){
               assert(false && "unsupported function parameter type");
           }
           AbstractState *as = stateMap.atAbstractState(ai);
           as -> setValueRange(input_ranges[i]);
           i ++;
      }
      for (auto &gv : F.getParent()->getGlobalList()){
                  if(gv.getType()->isPointerTy()){
                      PointerType *pt = cast<PointerType>(gv.getType());
                      if(pt -> getElementType() -> isArrayTy()){
                          ArrayType *at = cast<ArrayType>(pt -> getElementType());
                          vector<std::unique_ptr<AbstractState>> &as_array = stateMap.atAggregateState(&gv);
                          as_array.resize(at->getNumElements());
                          if(gv.hasInitializer()){
                              if (ConstantDataArray* initArray = dyn_cast<ConstantDataArray>(gv.getInitializer())){
                                  for(int i = 0; i < at -> getNumElements(); i ++){
                                      if(at -> getElementType() -> isIntegerTy()){
                                          ConstantInt *C = cast<ConstantInt>(initArray -> getAggregateElement(i));
                                          AbstractState *as = new AbstractState(C -> getType(),C -> getSExtValue(), C -> getSExtValue());
                        
                                          as_array[i] = unique_ptr<AbstractState>(as);
                                      }else{
                                          ConstantFP *C = cast<ConstantFP>(initArray -> getAggregateElement(i));
                                          double c;
                                          if(&(C -> getValueAPF().getSemantics()) == &(APFloat::IEEEsingle()))
                                              c = C->getValueAPF().convertToFloat();
                                          else
                                              c = C->getValueAPF().convertToDouble();
                                          AbstractState *as = new AbstractState(C -> getType(), c, c);
                                          as_array[i] = unique_ptr<AbstractState>(as);
                                      }
                                  }
                              }
                          }
                      }
                  } else if(gv.getType() -> isSingleValueType()) {
                      AbstractState *as = stateMap.atAbstractState(&gv);
                      if(gv.hasInitializer()){
                          ConstantFP *C = cast<ConstantFP>(gv.getInitializer());
                          double c;
                          if(&(C -> getValueAPF().getSemantics()) == &(APFloat::IEEEsingle()))
                              c = C->getValueAPF().convertToFloat();
                          else
                              c = C->getValueAPF().convertToDouble();
                          as -> setValueRange(ValueRange(c));
                      }
                  }
      }


    PerturbationMap pm;
    Executor executor(&F, stateMap, pm, VSet);
//    executor.run();
    executor.runPartialDifference();
                  
    return executor.getTargetEpsilon();
}

PreservedAnalyses CertInstCombinePass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
    FunctionPassManager FPM;
    //FPM.addPass(AnnoInstCombinePass());
    FPM.run(F, AM);
    
    // Mark all the analyses that instcombine updates as preserved.
    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    return PA;
}


char CertInstCombine::ID = 0;
static RegisterPass<CertInstCombine> X("certinstcombine", "Certified InstCombine Pass");



