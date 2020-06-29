add_llvm_loadable_module(LLVMAnnoInstCombine
  InstructionCombining.cpp
  InstCombineAddSub.cpp
  InstCombineAndOrXor.cpp
  InstCombineCalls.cpp
  InstCombineCasts.cpp
  InstCombineCompares.cpp
  InstCombineLoadStoreAlloca.cpp
  InstCombineMulDivRem.cpp
  InstCombinePHI.cpp
  InstCombineSelect.cpp
  InstCombineShifts.cpp
  InstCombineSimplifyDemanded.cpp
  InstCombineVectorOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/InstCombine

  PLUGIN_TOOL
  opt
  )
  
set_target_properties( LLVMAnnoInstCombine PROPERTIES COMPILE_FLAGS "-fno-rtti" )
set_target_properties( LLVMAnnoInstCombine PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
