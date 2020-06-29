add_llvm_loadable_module(LLVMCertInstCombine
    CertInstCombine.cpp
    Executor.cpp
    SplitInfoState.cpp
    ExecutionStateCalculation.cpp
    AbstractState.cpp
    ExecutionStateMap.cpp
    SplitStrategy.cpp
    IntervalParser.cpp
    PartialDiffExecutor.cpp

PLUGIN_TOOL
opt
 
DEPENDS
LLVMAnnoInstCombine
)

set_target_properties( LLVMCertInstCombine PROPERTIES COMPILE_FLAGS "-fno-rtti" )
set_target_properties( LLVMCertInstCombine PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
