//
//  Perturbation.h
//  Project
//
//  Created by Gu Yijia on 10/27/17.
//

#ifndef Perturbation_h
#define Perturbation_h

#include <map>
#include <cmath>
#include <string>

#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "perturbation"

namespace llvm {
    
    template <typename T>
    class PerturbationTy{
    public:
        void compositionAdd(const PerturbationTy& p, IRBuilder<> &builder){
            for(auto d : p.deltas_){
                if(deltas_.find(d.first) != deltas_.end()){
                    for(int i = 0; i < deltas_[d.first].size(); i ++){
                        deltas_[d.first][i] = builder.CreateFAdd((d.second)[i], deltas_[d.first][i]);
                    }
                    
                } else {
                    deltas_[d.first] = d.second;
                }
            }
        }
        
        void compositionMult(Value *C, IRBuilder<> &builder){
            for(auto d : deltas_){
                for(int i = 0; i < deltas_[d.first].size(); i ++){
                    deltas_[d.first][i] = builder.CreateFMul((d.second)[i], C);
                }
            }
        }
        
        
        std::map<Value*,  std::vector<Value*> > getDeltas() const {return deltas_;}
        
        void insertCommAndAsso(Value *X, Value *Z,  Value *ori_inst, IRBuilder<> &builder){
            //P  : = ( Y − Z ) · δ
            insert(builder.CreateFSub(X, Z), ori_inst);
        }
        
        void insertConstCommAndAsso(Value *X, ConstantFP *c1,  ConstantFP *c2, Value *ori_inst, IRBuilder<> &builder){
            //P = ( X − c 2 ) · δ 1 + ( c 1 + c 2 ) · ( 1 − ε / u ) · δ 2
            insert(builder.CreateFSub(X, c2), ori_inst);
            
            T FpC1 = getFPvalue(c1);
            T FpC2 = getFPvalue(c2);
            T t = FpC1 + FpC2 - twoSum(FpC1, FpC2)/ulp;
            Constant *e = ConstantFP::get(X->getType(), t);
            insert(e, ori_inst);
        }
        
        void insertFactor(Value *X, Value *Z, Instruction::BinaryOps op, Value *ori_inst, IRBuilder<> &builder){
            //p : = 2 X Y · δ
            BinaryOperator *t, *d;
            Constant *two = ConstantFP::get(X -> getType(), 2.0f);
            t = cast<BinaryOperator>(builder.CreateFMul(two, Z));
            d = BinaryOperator::Create(op, t, X);
            builder.Insert(d);
            insert(d, ori_inst);
        }

        void insertConstFactor(Value *X, ConstantFP *c1, ConstantFP *c2, Instruction::BinaryOps Xop, Instruction::BinaryOps Iop,  Value *ori_inst, IRBuilder<> &builder){
            //P  := 2Xc1 · δ1 + X · (c1 + c2) · (1 − ε/u) · δ2, ε := rel(c1 + c2)

            Constant *two = ConstantFP::get(X -> getType(), 2.0f);
            BinaryOperator *twoC = (Iop == Instruction::FAdd)? BinaryOperator::CreateFMul(two, c1): BinaryOperator::CreateFMul(two, c2);
            builder.Insert(twoC);
            BinaryOperator *intru= BinaryOperator::Create(Xop, twoC, X);
            builder.Insert(intru);
            insert(intru, ori_inst);
            
            T FpC1 = getFPvalue(c1);
            T FpC2 = getFPvalue(c2);
            FpC2 = (Iop == Instruction::FAdd) ? FpC2 : -FpC2;
            
            T t = FpC1 + FpC2 - twoSum(FpC1, FpC2)/ulp;
            Constant *e = ConstantFP::get(X->getType(), t);
            intru = BinaryOperator::Create(Xop, e, X);
            builder.Insert(intru);
            insert(intru, ori_inst);
        }
        
        void insertMDC(Value *X, ConstantFP *c1, ConstantFP *c2, Instruction::BinaryOps Xop,
                       Instruction::BinaryOps Cop, bool isCNumerator, Value *ori_inst, IRBuilder<> &builder){
            T FpC1 = getFPvalue(c1);
            T FpC2 = getFPvalue(c2);
            //TODO: Assume no underflow and overflow, MDC has associativity property
            if(isFPPowerOfTwo(FpC1) && isFPPowerOfTwo(FpC2))
                return;
            
            BinaryOperator *cb = BinaryOperator::Create(Cop, c1, c2);
            BinaryOperator *xb = isCNumerator? BinaryOperator::Create(Xop, cb, X):BinaryOperator::Create(Xop, X, cb);
            builder.Insert(cb);
            builder.Insert(xb);
            
            T t = (Cop == Instruction::FMul) ? twoMultFMA(FpC1, FpC2)/ulp : twoDivRes(FpC1, FpC2)/ulp;
            
            if(isCNumerator){
                Constant *e = ConstantFP::get(X->getType(), t);
                BinaryOperator *epsilonOp = BinaryOperator::Create(Xop, e, X);
                builder.Insert(epsilonOp);
                insert(builder.CreateFSub(xb, epsilonOp), ori_inst);
            }else{//C is a denominator
                t = (Cop == Instruction::FMul) ?  t / (FpC1 * FpC2)  : t / (FpC1 / FpC2);
                Constant *e = ConstantFP::get(X->getType(), t);
                BinaryOperator *epsilonOp = cast<BinaryOperator>(builder.CreateFMul(xb, e));
                insert(builder.CreateFSub(xb, epsilonOp), ori_inst);
            }
        }
        
        void insertReciprocal(Value *X, ConstantFP *c, Value *ori_inst, IRBuilder<> &builder){
            T FpC = getFPvalue(c);
            T t = twoDivRes(1, FpC)/ulp;
            
            Constant *e = ConstantFP::get(X->getType(), t);
            Value *v = builder.CreateFMul(e, X);

            insert(v, ori_inst);
        }
        
        void insertLog2OfHalf(){
            
        }
        
        void insertMulDivideConst(ConstantFP *c1, ConstantFP *c2, Value *ori_inst, IRBuilder<> &builder){
            //P  := c1c · (1 − ε/u) · δ,ε = rel(c1c)
            T FpC1 = getFPvalue(c1);
            T FpC2 = getFPvalue(c2);
            
            //using 2MultFMA to extract rounding error in C1 * C2
            T r = FpC1*FpC2 - twoMultFMA(FpC1, FpC2)/ulp;
            
            Constant *e = ConstantFP::get(c1 -> getType(), r);
            insert(e, ori_inst);
        }
        
        
    private:
        std::map<Value*, std::vector<Value*> > deltas_;
        
#ifdef SINGLE_FP_TYPE
        double ulp = std::numeric_limits<float>::epsilon();
#else
        double ulp = std::numeric_limits<double>::epsilon();
#endif
        
        void insert(Value *V, Value *ori_inst){
            if(deltas_.find(ori_inst) == deltas_.end()){
                 deltas_[ori_inst] = std::vector<Value*>();
            }
           
            deltas_[ori_inst].push_back(V);
            
//            if(Instruction *I = dyn_cast<Instruction> (ori_inst)){
//                I->print(dbgs());
//            }
        }
        
        /// use 2sum to extract rounding error, see 'Handbook of FP' P130
        T twoSum(const T a, const T b) const{
            T s = a + b;
            T a1 = s - b;
            T b1 = s - a1;
            T delta_a = a - a1;
            T delta_b = b - b1;
            return fabs(delta_a + delta_b);
        }
        
        /// using 2MultFMA to extract rounding error in a * b
        T twoMultFMA(const T a, const T b) const{
            T r1 = a * b;
            return fabs(fma(a, b, -r1));
        }
        /// extract the residual of a/b, see 'FP handbook' P153
        T twoDivRes(const T a, const T b) const{
            T r1 = a/b;
            return fabs(fma(r1, b, -a))/b;
        }
        
        T getFPvalue(ConstantFP *c) const;
        
        bool isFPPowerOfTwo(T c) const;
    };
    
    
    template <>
    inline float PerturbationTy<float>::getFPvalue(ConstantFP *c) const{
        return c->getValueAPF().convertToFloat();
    }
    
    template <>
    inline double PerturbationTy<double>::getFPvalue(ConstantFP *c) const{
        return c->getValueAPF().convertToDouble();
    }
    
    template<typename T>
    inline T PerturbationTy<T>::getFPvalue(ConstantFP *c) const{
        assert(false && "unexpected type for getFPValue");
        return c->getValueAPF().convertToDouble();
    }
    
    template<typename T>
    inline bool PerturbationTy<T>::isFPPowerOfTwo(T c) const{
        int i;
        T f = std::frexp(c, &i);
        return (f == 0.5) || (f == -0.5);
    }

    
#ifdef SINGLE_FP_TYPE
    using Perturbation = PerturbationTy<float>;
    using PerturbationMap = std::map<Value *, Perturbation>;
#else
    using Perturbation = PerturbationTy<double>;
    using PerturbationMap = std::map<Value *, Perturbation>;
#endif
    
}
#endif /* Perturbation_h */
