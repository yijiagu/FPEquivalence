add_llvm_loadable_module(LLVMLoopVectorize
LoopVectorize.cpp

PLUGIN_TOOL
opt
 
)


set_target_properties( LLVMLoopVectorize PROPERTIES COMPILE_FLAGS "-fno-rtti" )
set_target_properties( LLVMLoopVectorize PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
