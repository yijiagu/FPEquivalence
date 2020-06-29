#include <limits>
#include <queue>
#include <unordered_set>
#include <stack>

#include "Executor.h"
#include "ExecutionStateCalculation.h"
#include "PartialDiffExecutor.h"

#include "llvm/Support/Debug.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"

using namespace std;
using namespace llvm;
using namespace raic;

#define DEBUG_TYPE "executor"

void Executor::run(){
    llvm::SmallVector<llvm::Instruction *, 256> ori_inst_list;
     llvm::SmallVector<llvm::Instruction *, 256> modified_inst_list;
     pair<Value*, Value*> diff_pair;
     bool isOriInst = true;
     for (Instruction &I : fp_->getEntryBlock()){
        if(isa<CallInst>(&I)){
            CallInst *ci = cast<CallInst>(&I);
            StringRef fnName = ci -> getCalledFunction() -> getName();
            if(fnName == "compFusion"){
                diff_pair = make_pair(I.getOperand(0), I.getOperand(1));
                continue;
            }else if(fnName == "separate"){
                isOriInst = false;
                continue;
            }
        }
        
        if(isOriInst){
            ori_inst_list.push_back(&I);
        }else{
            modified_inst_list.push_back(&I);
        }
    }
     
    worklist = ori_inst_list;
    while(!worklist.empty()){ //compute the FP error of the original program
        auto I = worklist.begin();
        DEBUG(dbgs() << "visit inst:" << **I << "\n");
        visit(*I);
        worklist.erase(I);
     }
     
     worklist = modified_inst_list;
     while(!worklist.empty()){ //compute the FP error of the original program
         auto I = worklist.begin();
         DEBUG(dbgs() << "visit inst:" << **I << "\n");
         visit(*I);
         worklist.erase(I);
     }
    
     
     AbstractState *abstract_ori = value_state_map_.atAbstractState(diff_pair.first);
     AbstractState *abstract_modified = value_state_map_.atAbstractState(diff_pair.second);
    
     targetAs = *abstract_ori;
     
     AbstractState diffState;
     diffState.taylor_form = abstract_ori -> taylor_form;
       
     for(auto i : abstract_modified -> taylor_form){
        diffState.taylor_form[i.first] = diffState.taylor_form[i.first] - i.second;
     }
    
     targetAbsDiff = ValueRange(diffState.getMinError(),  diffState.getMaxError());
}

void Executor::runPartialDifference(){
    llvm::SmallVector<llvm::Instruction *, 256> inst_list;
    pair<Value*, Value*> diff_pair = findPartialDiffInstr(&fp_->getEntryBlock(), inst_list);
    worklist = inst_list;
    while(!worklist.empty()){ //compute the FP error of the original program
        auto I = worklist.begin();
        DEBUG(dbgs() << "visit inst:" << **I << "\n");
        visit(*I);
        worklist.erase(I);
    }
    
    SmallVector<pair<Instruction *, Value*>, 256> common_inst_list;
    pair<Value*, Value*> root_diff_pair = findMaxCommonChain(diff_pair.first, diff_pair.second, common_inst_list);
    
    for(auto iter : common_inst_list){
        DEBUG(dbgs() << "visit comm inst:" << *iter.first << "\n");
    }
    
    worklist = buildInstListOfDiff(root_diff_pair.second);
    
    while(!worklist.empty()){ //compute the FP error of the original program
        auto I = worklist.begin();
        DEBUG(dbgs() << "visit inst:" << **I << "\n");
        visit(*I);
        worklist.erase(I);
    }
    
    targetAs = *(value_state_map_.atAbstractState(diff_pair.first));
    
    auto pd = PartialDiffExecutor(root_diff_pair.first, root_diff_pair.second, value_state_map_, common_inst_list);
    pd.run();
    targetAbsDiff = pd.getTargetAbsDiff();
}

void Executor::visitSExt(llvm::SExtInst &I){
    DEBUG(dbgs() << "visit SExt" << "\n");
    value_state_map_.copyValue(I.getOperand(0), &I);
}

void Executor::visitShl(llvm::BinaryOperator &I){
    AbstractState *abstract_A = value_state_map_.atAbstractState(I.getOperand(0));
    AbstractState *abstract_B = value_state_map_.atAbstractState(I.getOperand(1));
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    assert((abstract_A -> getValueRange().lower() == abstract_A -> getValueRange().upper()) && "Ambiguous Integer Operation");
    assert((abstract_B -> getValueRange().lower() == abstract_B -> getValueRange().upper()) && "Ambiguous Integer Operation");
    
    int i = abstract_A -> getValueRange().lower();
    int s = abstract_B -> getValueRange().lower();
    i = i << s;
    abstract_I -> setValueRange(ValueRange(i));
}

void Executor::visitAdd(BinaryOperator &I){
    DEBUG(dbgs() << "visit Add" << "\n");
    AbstractState *abstract_A = value_state_map_.atAbstractState(I.getOperand(0));
    AbstractState *abstract_B = value_state_map_.atAbstractState(I.getOperand(1));
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    ValueRange vr =  abstract_A -> getValueRange() + abstract_B -> getValueRange();
    abstract_I -> clearState();
    abstract_I -> setValueRange(vr);
}

void Executor::visitSub(BinaryOperator &I){
    DEBUG(dbgs() << "visit Sub" << "\n");
    AbstractState *abstract_A = value_state_map_.atAbstractState(I.getOperand(0));
    AbstractState *abstract_B = value_state_map_.atAbstractState(I.getOperand(1));
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    ValueRange vr =  abstract_A -> getValueRange() - abstract_B -> getValueRange();
    abstract_I -> clearState();
    abstract_I -> setValueRange(vr);
    
}

void Executor::visitMul(BinaryOperator &I){
    DEBUG(dbgs() << "visit Mul" << "\n");
    AbstractState *abstract_A = value_state_map_.atAbstractState(I.getOperand(0));
    AbstractState *abstract_B = value_state_map_.atAbstractState(I.getOperand(1));
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    ValueRange vr =  abstract_A -> getValueRange() * abstract_B -> getValueRange();
    abstract_I -> clearState();
    abstract_I -> setValueRange(vr);
    
}


void Executor::visitOr(llvm::BinaryOperator &I){
    AbstractState *abstract_A = value_state_map_.atAbstractState(I.getOperand(0));
    AbstractState *abstract_B = value_state_map_.atAbstractState(I.getOperand(1));
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    assert((abstract_A -> getValueRange().lower() == abstract_A -> getValueRange().upper()) && "Ambiguous Integer Operation");
    assert((abstract_B -> getValueRange().lower() == abstract_B -> getValueRange().upper()) && "Ambiguous Integer Operation");
    
    int i = abstract_A -> getValueRange().lower();
    int s = abstract_B -> getValueRange().lower();
    i = i | s;
    abstract_I -> setValueRange(ValueRange(i));
}

void Executor::visitAnd(llvm::BinaryOperator &I){
    AbstractState *abstract_A = value_state_map_.atAbstractState(I.getOperand(0));
    AbstractState *abstract_B = value_state_map_.atAbstractState(I.getOperand(1));
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    assert((abstract_A -> getValueRange().lower() == abstract_A -> getValueRange().upper()) && "Ambiguous Integer Operation");
    assert((abstract_B -> getValueRange().lower() == abstract_B -> getValueRange().upper()) && "Ambiguous Integer Operation");
    
    int i = abstract_A -> getValueRange().lower();
    int s = abstract_B -> getValueRange().lower();
    i = i & s;
    abstract_I -> setValueRange(ValueRange(i));
}

void Executor::visitFAdd(BinaryOperator &I){
    DEBUG(dbgs() << "visit FAdd" << "\n");
    
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    
    Type *ty = A -> getType();
    if(ty -> isVectorTy()){//handle the vector type
        auto& abstract_A = value_state_map_.atAggregateState(A);
        auto& abstract_B = value_state_map_.atAggregateState(B);
        auto& abstract_I = value_state_map_.atAggregateState(&I);
        abstract_I.clear();
        for(int i = 0; i < ty -> getVectorNumElements(); i ++){
            abstract_I.push_back(make_unique<AbstractState>());
            *abstract_I[i] = *abstract_A[i] + *abstract_B[i];
        }
        
        //handle reduction addition
        if(reduce_I == A){
            for(int i = 0; i < abstract_B.size(); i ++){
                *accumulated = *accumulated + *abstract_B[i];
            }
            reduce_I = &I;
        }else if(reduce_I == B){
            for(int i = 0; i < abstract_A.size(); i ++){
                *accumulated = *accumulated + *abstract_A[i];
            }
            reduce_I = &I;
        }
        return;
    }
    
    
    AbstractState *abstract_A = value_state_map_.atAbstractState(A);
    AbstractState *abstract_B = value_state_map_.atAbstractState(B);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    *abstract_I = (*abstract_A) + (*abstract_B);
    
}

void Executor::visitFSub(BinaryOperator &I){
    DEBUG(dbgs() << "visit FSub" << "\n");
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    
    AbstractState *abstract_A = value_state_map_.atAbstractState(A);
    AbstractState *abstract_B = value_state_map_.atAbstractState(B);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    *abstract_I = (*abstract_A) - (*abstract_B);
}

void Executor::visitFMul(BinaryOperator &I){
    DEBUG(dbgs() << "visit FMul" << "\n");
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
   
    Type *ty = A -> getType();
    if(ty -> isVectorTy()){//handle the vector type
        auto& abstract_A = value_state_map_.atAggregateState(A);
        auto& abstract_B = value_state_map_.atAggregateState(B);
        auto& abstract_I = value_state_map_.atAggregateState(&I);
        abstract_I.clear();
        for(int i = 0; i < ty -> getVectorNumElements(); i ++){
            abstract_I.push_back(make_unique<AbstractState>());
            *abstract_I[i] = *abstract_A[i] * *abstract_B[i];
        }
        
        //handle reduction addition
        if(reduce_I == A){
            for(int i = 0; i < abstract_B.size(); i ++){
                *accumulated = *accumulated * *abstract_B[i];
            }
            reduce_I = &I;
        }else if(reduce_I == B){
            for(int i = 0; i < abstract_A.size(); i ++){
                *accumulated = *accumulated * *abstract_A[i];
            }
            reduce_I = &I;
        }
        return;
    }
    
    AbstractState *abstract_A = value_state_map_.atAbstractState(A);
    AbstractState *abstract_B = value_state_map_.atAbstractState(B);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    *abstract_I = (*abstract_A) * (*abstract_B);
}

void Executor::visitFDiv(BinaryOperator &I){
    DEBUG(dbgs() << "visit FDiv" << "\n");
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    
    AbstractState *abstract_A = value_state_map_.atAbstractState(A);
    AbstractState *abstract_B = value_state_map_.atAbstractState(B);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    *abstract_I = (*abstract_A) / (*abstract_B);
}


void Executor::visitExtractElementInst (llvm::ExtractElementInst &I){
    DEBUG(dbgs() << "visit ExtractElement" << "\n");
    Value *A = I.getVectorOperand();
    Value *B = I.getIndexOperand();
    
    auto& abstract_A = value_state_map_.atAggregateState(A);
    AbstractState *abstract_B = value_state_map_.atAbstractState(B);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);

    abstract_I -> clearState();
    *abstract_I = *abstract_A[(int)abstract_B->getValueRange().lower()];

    //compute wilkinson number
    //*abstract_I = computeWilkinson(*abstract_I, *accumulated);
    *accumulated = AbstractState();
}

void Executor::visitInsertElementInst (llvm::InsertElementInst &I){
  
    Type *ty = I.getType();
    
    Value *vec = I.getOperand(0);
    Value *scalar = I.getOperand(1);
    Value *index = I.getOperand(2);
    
    AbstractState *abstract_scalar = value_state_map_.atAbstractState(scalar);
    AbstractState *abstract_index = value_state_map_.atAbstractState(index);
    auto& abstract_I = value_state_map_.atAggregateState(&I);
    abstract_I.clear();
    for(int i = 0; i < ty -> getVectorNumElements(); i ++){
        abstract_I.push_back(make_unique<AbstractState>());
    }
    
    //TODO: handle undef
    if(!isa<Constant>(vec)){
        auto& abstract_vec = value_state_map_.atAggregateState(vec);
        for(int i = 0; i < ty -> getVectorNumElements(); i ++){
             *abstract_I[i] = *abstract_vec[i];
        }
    }
    
    int index_val  = abstract_index->getValueRange().lower();
    *abstract_I[index_val] = *abstract_scalar;
}

void Executor::visitShuffleVectorInst (llvm::ShuffleVectorInst &I){
    DEBUG(dbgs() << "visit ShuffleVector" << "\n");
    Value *A = I.getOperand(0);
    Value *B = I.getOperand(1);
    
    if(reduce_I == A || reduce_I == B){ //we are now doing the reduction
        reduce_I = nullptr;
    }

    Type *ty = I.getType();
    Type *ty_A = A -> getType();

    auto& abstract_A = value_state_map_.atAggregateState(A);
    auto& abstract_B = value_state_map_.atAggregateState(B);

    auto& abstract_I = value_state_map_.atAggregateState(&I);
    abstract_I.clear();
    for(int i = 0; i < ty -> getVectorNumElements(); i ++){
        abstract_I.push_back(make_unique<AbstractState>());
    }
    
    int index = 0;

    for(auto i : I.getShuffleMask()){
        if(i == -1){ //ignore undef mask values
            continue;
        }

        if(i < ty_A -> getVectorNumElements()){
            *abstract_I[index] = *abstract_A[i];
        }else{
            *abstract_I[index] = *abstract_B[i - ty_A -> getVectorNumElements()];
        }

        index ++;
    }
    
}

void Executor::visitGetElementPtrInst (llvm::GetElementPtrInst &I){
    DEBUG(dbgs() << "visit GEP" << "\n");
    PointerIndex pi = computeGEP(dyn_cast<GEPOperator>(&I));
    value_state_map_.update(&I, pi);
}

void Executor::visitStoreInst(StoreInst &SI){
    DEBUG(dbgs() << "visit StoreInst" << "\n");
    Value* pointer = SI.getPointerOperand();
    if(ConstantExpr *ceP = dyn_cast<ConstantExpr>(pointer)){
        if(isa<GEPOperator>(ceP)){
            PointerIndex pi = computeGEP(dyn_cast<GEPOperator>(ceP));
            value_state_map_.update(ceP, pi);
            value_state_map_.copyValue(SI.getValueOperand(), ceP);
        }else{
          assert(false && "error");
        }
    } else {
        value_state_map_.copyValue(SI.getValueOperand(), pointer);
    }
}

void Executor::visitLoadInst(LoadInst &LI){
    if(LI.getType() -> isVectorTy()){ //handling vector
        Type *ty = LI.getType();
        Value *pointer = LI.getPointerOperand();
        auto& abstract_I = value_state_map_.atAggregateState(&LI);
        abstract_I.clear();
        for(int i = 0; i < ty -> getVectorNumElements(); i ++){
            abstract_I.push_back(make_unique<AbstractState>());
        }
        PointerIndex pi;
        if(value_state_map_.hasPointerIndex(pointer, pi)){
            auto& abstract_A = value_state_map_.atAggregateState(pi.base);
            for(int i = 0; i < ty -> getVectorNumElements(); i ++){
                *abstract_I[i] = *abstract_A[pi.index + i];
            }
        }else{
            assert(false && "load null ptr");
        }
        
        if (MDNode* N = LI.getMetadata("reduce.add")){
            *accumulated = *abstract_I[0];
            reduce_I = &LI;
            for(int i = 1; i < ty -> getVectorNumElements(); i ++){
                *accumulated = *accumulated + *abstract_I[i];
            }
        }
        
        return;
    }
    Value* pointer = LI.getPointerOperand();
    if(isa<ConstantExpr>(pointer)){
        PointerIndex pi = computeGEP(dyn_cast<GEPOperator>(pointer));
        value_state_map_.update(pointer, pi);
        value_state_map_.copyValue(pointer, &LI);
    } else {
        value_state_map_.copyValue(pointer, &LI);
    }
}

void Executor::visitAllocaInst(AllocaInst &I){
    Type *ty = I.getAllocatedType();
    if(ty -> isFloatingPointTy()){
        value_state_map_.atAbstractState(&I);
    }else if(ty -> isIntegerTy()){
        //Do nothing
    }else if(ty -> isAggregateType()){
        vector<std::unique_ptr<AbstractState>> &as_array = value_state_map_.atAggregateState(&I);
        for(int i = 0; i < getAbstractSize(ty); i++){//initialize abstract state
            as_array.push_back(unique_ptr<AbstractState>(new AbstractState()));
        }
        PointerIndex pi;
        pi.base = &I;
        pi.index = 0;
        value_state_map_.update(&I, pi);
    }else if(ty -> isPointerTy()){ //treat pointerTy as aggregate with 1 element
        vector<std::unique_ptr<AbstractState>> &as_array = value_state_map_.atAggregateState(&I);
        as_array.push_back(unique_ptr<AbstractState>(new AbstractState()));
        PointerIndex pi;
        pi.base = &I;
        pi.index = 0;
        value_state_map_.update(&I, pi);
    }else {
        assert(false && "unsupported alloc type");
    }

}

void Executor::visitIntrinsic(IntrinsicInst &II){
    //TODO: handle intrinsics
}

void Executor::visitReturnInst(ReturnInst &I){
    DEBUG(dbgs() << "visit Return instruction" << "\n");
    targetAs = *value_state_map_.atAbstractState(I.getReturnValue());
}

void Executor::visitBranchInst(BranchInst &I){
    if (I.isUnconditional()){
        DEBUG(dbgs() << "Unconditional branch: " << I << "\n") ;
        curr_executable_edge_ = make_pair(I.getParent(),I.getSuccessor(0));
        addBlockInstToWorklist(I.getSuccessor(0));
        return;
    } // End Unconditional Branch
    
    DEBUG(dbgs() << "Conditional branch: " << I << "\n") ;
    
    // Special cases if constants true or false
    if (value_state_map_.isTrueConstant(I.getCondition())){
        DEBUG(dbgs() << "\tthe branch condition is MUST BE TRUE.\n") ;
        curr_executable_edge_ = make_pair(I.getParent(),I.getSuccessor(0));
        addBlockInstToWorklist(I.getSuccessor(0));
        return;
    }
    
    if (value_state_map_.isFalseConstant(I.getCondition())){
        DEBUG(dbgs() << "\tthe branch condition is MUST BE FALSE.\n") ;
        curr_executable_edge_ = make_pair(I.getParent(),I.getSuccessor(1));
        addBlockInstToWorklist(I.getSuccessor(1));
        return;
    }
    
    assert(false && "undetermined branch instruction");
}
void Executor::visitFCmpInst(FCmpInst &I){
     DEBUG(dbgs() << "visit FCmp instruction" << "\n");
     switch(I.getPredicate()){
         case CmpInst::FCMP_FALSE:
             break;
         case CmpInst::FCMP_OEQ: {
             AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
             AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
             
             if(op1->getValueRange() == op2 -> getValueRange()){
                 value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
             }else{
                 value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
             }
             break;
         }
         case CmpInst::FCMP_OGT: case CmpInst::FCMP_UGT:{
             AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
             AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
             
             if(op1->getValueRange() > op2 -> getValueRange()){
                 value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
             }else{
                 value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
             }
             break;
         }
         case CmpInst::FCMP_OGE:{
             AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
             AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
             
             if(op1->getValueRange() >= op2 -> getValueRange()){
                 value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
             }else{
                 value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
             }
             break;
         }

         case CmpInst::FCMP_OLT: case CmpInst::FCMP_ULT: {
             AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
             AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
             
             if(op1->getValueRange() < op2 -> getValueRange()){
                 value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
             }else{
                 value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
             }
             break;
         }
            
         case CmpInst::FCMP_OLE:{
             AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
             AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
             
             if(op1->getValueRange() <= op2 -> getValueRange()){
                 value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
             }else{
                 value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
             }
             break;
         }
             
         case CmpInst::FCMP_ONE:
             assert(false && "unsupported ICmp instruction");
             break;
         case CmpInst::FCMP_UNE:{
             AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
             AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
             
             if(boost::numeric::empty(boost::numeric::intersect(op1->getValueRange(),  op2 -> getValueRange()))){
                 value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
             }else{
                 value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
             }
             break;
         }
         case CmpInst::FCMP_ORD:
             assert(false && "unsupported ICmp instruction");
             break;
         case CmpInst::FCMP_TRUE:
             assert(false && "unsupported ICmp instruction");
             break;
         default:
             assert(false && "unsupported ICmp instruction");
     }
}
void Executor::visitICmpInst(ICmpInst &I){
    DEBUG(dbgs() << "visit ICmp instruction" << "\n");
    //TODO : assuem signed predicate
    switch(I.getSignedPredicate()){
        
        case CmpInst::ICMP_EQ:{
            
            AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
            AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
            
            if(op1->getValueRange() == op2 -> getValueRange()){
                value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
            }else{
                value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
            }
            break;
        }
        case CmpInst::ICMP_NE:{
            AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
            AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
            
            if(op1->getValueRange() == op2 -> getValueRange()){
                value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
            }else{
                value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
            }
            break;
        }
        case CmpInst::ICMP_SGT:{
            AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
            AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
            
            if(op1->getValueRange() > op2 -> getValueRange()){
                value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
            }else{
                value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
            }
            break;
        }
        case CmpInst::ICMP_SGE:{
            AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
            AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
            
            if(op1->getValueRange() >= op2 -> getValueRange()){
                value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
            }else{
                value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
            }
            break;
        }
        case CmpInst::ICMP_SLT:{
            AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
            AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
            
            if(op1->getValueRange() < op2 -> getValueRange()){
                value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
            }else{
                value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
            }
            break;
        }
        case CmpInst::ICMP_SLE:{
            AbstractState *op1 =  value_state_map_.atAbstractState(I.getOperand(0));
            AbstractState *op2 =  value_state_map_.atAbstractState(I.getOperand(1));
            
            if(op1->getValueRange() <= op2 -> getValueRange()){
                value_state_map_.update(&I, ConstantInt::getTrue(I.getType()));
            }else{
                value_state_map_.update(&I, ConstantInt::getFalse(I.getType()));
            }
            break;
        }
        default:
            assert(false && "unsupported ICmp instruction");
    }
}

void Executor::visitPHINode(PHINode &I){
    DEBUG(dbgs() << "PHI node " << I << "\n");
    for (unsigned i = 0, num_vals = I.getNumIncomingValues(); i != num_vals;i++) {
        // if (!isEdgeFeasible(PN.getIncomingBlock(i), PN.getParent()))
        //   dbgs() << PN.getIncomingBlock(i)->getName()
        //          << " --> " << PN.getParent()->getName()  << " IS UNREACHABLE\n";
        // else{
        //   dbgs() << PN.getIncomingBlock(i)->getName()
        //          << " --> " << PN.getParent()->getName()  << " IS REACHABLE\n";
        // }
        
        if(I.getIncomingBlock(i) == curr_executable_edge_.first){
            if(I.getType() -> isVectorTy()){ //handling vector
                Type *ty = I.getType();
                Value *pointer = I.getIncomingValue(i);
                
                auto& abstract_incoming = value_state_map_.atAggregateState(pointer);
                auto& abstract_I = value_state_map_.atAggregateState(&I);
                abstract_I.clear();
                for(int i = 0; i < ty -> getVectorNumElements(); i ++){
                    abstract_I.push_back(make_unique<AbstractState>());
                }
                
                for(int i = 0; i < ty -> getVectorNumElements(); i ++){
                    *abstract_I[i] = *abstract_incoming[i];
                }
                
//                PointerIndex pi;
//                if(value_state_map_.hasPointerIndex(pointer, pi)){
//                    auto& abstract_A = value_state_map_.atAggregateState(pi.base);
//                    for(int i = 0; i < ty -> getVectorNumElements(); i ++){
//                        *abstract_I[i] = *abstract_A[pi.index + i];
//                    }
//                }else if(Constant* CP = dyn_cast<Constant>(pointer)){
//                    if(!CP -> isZeroValue()){
//                        assert(false && "fail to initialize phi node");
//                    }
//                }else{
//                    assert(false && "load null ptr");
//                }
                
                if (MDNode* N = I.getMetadata("reduction")){
                    if(reduce_I == nullptr){
                        StringRef reduction_type = cast<MDString>(N->getOperand(0))->getString();
                        if(reduction_type == "add"){
                            *accumulated = *abstract_I[0];
                        }else if(reduction_type == "mult"){
                            *accumulated = *abstract_I[0];
                        }
                    
                    }
                    reduce_I = &I;
                }
                return;
            }
            
            
            
            Value* v = I.getIncomingValue(i);
            value_state_map_.copyValue(v, &I);
           
            break;
        }
        
    }
}

void Executor::visitSwitchInst(llvm::SwitchInst  &I){
    AbstractState *cond = value_state_map_.atAbstractState(I.getCondition());
    assert((cond -> getValueRange().lower() == cond -> getValueRange().upper()) && "Ambiguous Switch Condition");
    
    int i = cond -> getValueRange().lower();
    curr_executable_edge_ = make_pair(I.getParent(),I.getSuccessor(i));
    addBlockInstToWorklist(I.getSuccessor(i));
}

void Executor::visitSIToFP(llvm::SIToFPInst &I){
    DEBUG(dbgs() << "visit SIToFP" << "\n");
    value_state_map_.copyValue(I.getOperand(0), &I);
}

void Executor::visitBitCastInst(llvm::BitCastInst &I){
    PointerIndex pi;
    if(value_state_map_.hasPointerIndex(I.getOperand(0), pi)){
        value_state_map_.update(&I, pi);
    }else{
        value_state_map_.copyValue(I.getOperand(0), &I);
    }
}

void Executor::visitZExtInst(llvm::ZExtInst &I){
    value_state_map_.copyValue(I.getOperand(0), &I);
}

void Executor::visitSelectInst(llvm::SelectInst &I){
    if (value_state_map_.isTrueConstant(I.getCondition())){
        value_state_map_.copyValue(I.getTrueValue(), &I);
    }else{
        value_state_map_.copyValue(I.getFalseValue(), &I);
    }
}

void Executor::visitCallInst(CallInst &I){
    StringRef fnName = I.getCalledFunction()->getName();
    
    if(fnName == "llvm.dbg.declare" || fnName == "llvm.dbg.value"){
        return;
    }
    
    Value *A = I.getOperand(0);
    AbstractState *abstract_A = value_state_map_.atAbstractState(A);
    AbstractState *abstract_I = value_state_map_.atAbstractState(&I);
    
    //clear the state of abstract_I for looping scenario
    abstract_I -> clearState();
    
    
    if(fnName == "sin") {
        *abstract_I = sin(*abstract_A);
        //ExecutionStateCalculation::updateSinExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "exp"){
        *abstract_I = exp(*abstract_A);
        //ExecutionStateCalculation::updateExpExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "cos") {
        *abstract_I = cos(*abstract_A);
        //ExecutionStateCalculation::updateCosExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "tan") {
        *abstract_I = tan(*abstract_A);
        //ExecutionStateCalculation::updateTanExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "atan2"){
        Value *B = I.getOperand(1);
        AbstractState *abstract_B = value_state_map_.atAbstractState(B);
        
        *abstract_I = atan2(*abstract_A, *abstract_B);
        
        //ExecutionStateCalculation::updateAtan2ExecutionState(&I, I.getOperand(0), I.getOperand(1), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "asin"){
        *abstract_I = asin(*abstract_A);
        //ExecutionStateCalculation::updateAsinExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "log"){
        *abstract_I = log(*abstract_A);
        //ExecutionStateCalculation::updateLogExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "llvm.sqrt.f64"){
        *abstract_I = sqrt(*abstract_A);
        //ExecutionStateCalculation::updateSqrtExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else if(fnName == "llvm.pow.f64"){
        Value *B = I.getOperand(1);
        AbstractState *abstract_B = value_state_map_.atAbstractState(B);
        *abstract_I = natPow(*abstract_A, *abstract_B);
        //ExecutionStateCalculation::updateSqrtExecutionState(&I, I.getOperand(0), value_state_map_, perturbation_map_,VSet_);
    } else {
        assert(false && "Calling Unsupported Function");
    }
}

pair<Value*, Value*> Executor::findMaxCommonChain(Value *A, Value *B, SmallVector<pair<Instruction *, Value*>, 256> &inst_list){
    Instruction *IA = dyn_cast<Instruction> (A);
    Instruction *IB = dyn_cast<Instruction> (B);
    
    if(IA == nullptr || IB == nullptr){
        return make_pair(A, B);
    }
    
    if((IA -> isBinaryOp() && !IB -> isBinaryOp()) ||
        (IB -> isBinaryOp() && !IA -> isBinaryOp())){// if A and B have different operands, then they are the root of MCC
        return make_pair(A, B);
    }else if(IA -> isBinaryOp() && IB -> isBinaryOp()){// A and B are both binaryOp;
        if(IA -> getOpcode() != IB -> getOpcode()){
           return make_pair(A, B); // if A and B have different opcodes, they are the root of MCC
        }
       
        Value *opA0 = IA -> getOperand(0);
        Value *opA1 = IA -> getOperand(1);
       
        Value *opB0 = IB -> getOperand(0);
        Value *opB1 = IB -> getOperand(1);
        
        if((opA0 == opB0 && opA1 == opB1) ||
           (opA0 == opB1 && opA1 == opB0)){
            return make_pair(nullptr, nullptr); // A and B are floating-point equivalent
        }
        Value *diff_branchA = nullptr;
        Value *diff_branchB = nullptr;
        Value *commonOp = nullptr;
        if(opA0 == opB0){ //opA1 != opB1
            commonOp = opA0;
            diff_branchA = opA1;
            diff_branchB = opB1;
        }else if(opA0 == opB1){ //opA1 != opB0
            commonOp = opA0;
            diff_branchA = opA1;
            diff_branchB = opB0;
        }else if(opA1 == opB0){//opA0 != opB1
            commonOp = opA1;
            diff_branchA = opA0;
            diff_branchB = opB1;
        }else if(opA1 == opB1){//opA0 != opB0
            commonOp = opA1;
            diff_branchA = opA0;
            diff_branchB = opB0;
        }
        
        if(diff_branchA != nullptr){
            auto p = findMaxCommonChain(diff_branchA, diff_branchB, inst_list);
            inst_list.push_back(make_pair(IA, commonOp));
            return p;
        }else{
            return make_pair(A, B);
        }
    }else{ // A and B are both unaryOp;
        if(IA -> getOpcode() != IB -> getOpcode()){
            return make_pair(A, B); // if A and B have different opcodes, they are the root of MCC
        }
        Value *opA0 = IA -> getOperand(0);
        Value *opB0 = IB -> getOperand(0);
        
        auto p =  findMaxCommonChain(opA0, opB0, inst_list);
        inst_list.push_back(make_pair(IA, opA0));
        return p;
    }
}


void Executor::addBlockInstToWorklist(BasicBlock *BB){
    for (Instruction &I : *BB){
         worklist.push_back(&I);
    }
}

pair<Value*, Value*> Executor::findPartialDiffInstr(BasicBlock *BB, llvm::SmallVector<llvm::Instruction *, 256>  &inst_list){
    bool isOriInst = true;
    for (Instruction &I : *BB){
        if(isa<CallInst>(&I)){
            CallInst *ci = cast<CallInst>(&I);
            StringRef fnName = ci -> getCalledFunction() -> getName();
            if(fnName == "compFusion"){
                return make_pair(I.getOperand(0), I.getOperand(1));
            }else if(fnName == "separate"){
                isOriInst = false;
            }
        }
        if(isOriInst){
            inst_list.push_back(&I);
        }
    }
    
    assert(false && "Cannot find 'compFusion' function");
    return make_pair(nullptr, nullptr);
}


llvm::SmallVector<llvm::Instruction *, 256> Executor::buildInstListOfDiff(llvm::Value* A){
    Instruction *IA = cast<Instruction> (A);
    llvm::SmallVector<llvm::Instruction *, 256> inst_list;
    
    std::queue<Instruction *> Iq;
    std::unordered_set<Instruction *> Iset;
    std::stack<Instruction *>Istack;
    Iq.push(IA);
    
    while(!Iq.empty()){
        Instruction *I = Iq.front();
        Iq.pop();
        if(Iset.count(I) == 0){
            Iset.insert(I);
            Istack.push(I);
            if(I -> isBinaryOp()){
                if(Instruction *op0 = dyn_cast<Instruction>(I -> getOperand(0))){
                    Iq.push(op0);
                }
                if(Instruction *op1 = dyn_cast<Instruction>(I -> getOperand(1))){
                    Iq.push(op1);
                }
            }else{// I has unary operator
                if(Instruction *op0 = dyn_cast<Instruction>(I -> getOperand(0))){
                    Iq.push(op0);
                }
            }
        }
    }

    while(!Istack.empty()){
        inst_list.push_back(Istack.top());
        Istack.pop();
    }
    return inst_list;
}

int Executor::getAbstractSize(llvm::Type *ty){
    if(ty -> isSingleValueType()){
        return 1;
    }else if(ty -> isArrayTy()){
        return ty -> getArrayNumElements() * getAbstractSize(ty -> getArrayElementType());
    }else if(ty -> isStructTy()){
        int size = 0;
        for(int i = 0; i < ty -> getStructNumElements(); i ++){
            Type *elem_ty = ty -> getStructElementType(i);
            if(elem_ty -> isSingleValueType()){
                size ++;
            }else if(elem_ty -> isArrayTy()){
                size += getAbstractSize(elem_ty);
            }else if(elem_ty -> isStructTy()){
                size += getAbstractSize(elem_ty);
            }
        }
        return size;
        
    }else{
        assert(false && "unsupported type");
    }
}

PointerIndex Executor::computeGEP(llvm::GEPOperator *gep){
    PointerIndex pi;
    if(! value_state_map_.hasPointerIndex(gep -> getPointerOperand(), pi)){
        assert(false && "visit null pointer in GEP");
    }
    Type *ty = gep -> getPointerOperand()->getType();
    
    auto i = gep -> idx_begin();
    if(ConstantInt *C1 = dyn_cast<ConstantInt>(*i)){
        pi.index += getAbstractSize(ty) * (C1 -> getZExtValue());
    }else{
        AbstractState *abstract_index = value_state_map_.atAbstractState(*i);
        assert(abstract_index -> getValueRange().lower() == abstract_index -> getValueRange().upper() && "Ambiguous pointer index");
        pi.index += getAbstractSize(ty) * abstract_index -> getValueRange().lower();
    }
    if(ty -> isPointerTy()){
        Type *pTy = ty -> getPointerElementType();
        if(pTy -> isArrayTy()){
            if(ConstantInt *CI = dyn_cast<ConstantInt>(*(i + 1))){
                pi.index += getAbstractSize(pTy -> getArrayElementType()) * (CI -> getZExtValue());
            }else{
                AbstractState *abstract_index = value_state_map_.atAbstractState(*(i + 1));
                assert(abstract_index -> getValueRange().lower() == abstract_index -> getValueRange().upper() && "Ambiguous pointer index");
                pi.index += getAbstractSize(pTy -> getArrayElementType()) * abstract_index -> getValueRange().lower();
            }
        }else if(pTy -> isStructTy()){
            int elem_index = 0;
            if(ConstantInt *CI = dyn_cast<ConstantInt>(*(i + 1))){
                elem_index = CI -> getZExtValue();
            } else {
                AbstractState *abstract_index = value_state_map_.atAbstractState(*(i + 1));
                assert(abstract_index -> getValueRange().lower() == abstract_index -> getValueRange().upper() && "Ambiguous pointer index");
                elem_index = abstract_index -> getValueRange().lower();
            }
            
            for(int j = 0; j < elem_index; j ++){
                pi.index += getAbstractSize(pTy -> getStructElementType(j));
            }
        }
    }else{
        assert(false && "unsupported alloc type");
    }
    return pi;
}


