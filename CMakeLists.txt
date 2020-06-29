cmake_minimum_required(VERSION 3.3)

set (CMAKE_CXX_STANDARD 11)
set (CXX_STANDARD_REQUIRED ON)

#Set (LLVM_DIR "/Users/yijia/Tools/llvm_dev/build/lib/cmake/llvm")

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(AddLLVM)

find_package(Boost)

#find_package(GMP)
MESSAGE(STATUS "${GMP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS}")

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS}
                    ${GMP_INCLUDE_DIR}
                    ${Boost_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS}
                 ${GMP_LIBRARIES}
                 ${Boost_LIBRARY_DIRS})

option(analysisTySingle "Using Single Precision Floating-point type" OFF)

if(analysisTySingle)
#  add_definitions(-DSINGLE_FP_TYPE)
endif()

add_subdirectory(CertInstCombine)
add_subdirectory(StableInstCombine)


