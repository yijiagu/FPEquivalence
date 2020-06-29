add_llvm_loadable_module(LLVMStableInstCombine
StableInstCombinePass.cpp

PLUGIN_TOOL
opt
 
DEPENDS
#LLVMLoopVectorize
LLVMCertInstCombine
)


set_target_properties( LLVMStableInstCombine PROPERTIES COMPILE_FLAGS "-fno-rtti" )
set_target_properties( LLVMStableInstCombine PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
