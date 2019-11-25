cmake_minimum_required(VERSION 3.4)

#if (NOT DEFINED ENV{LLVM_HOME})
#    message(FATAL_ERROR "$LLVM_HOME is not defined")
#else ()
#    set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
#endif()

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})

include_directories(${LLVM_INCLUDE_DIRS})

link_directories(${LLVM_LIBRARY_DIRS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(obfs)
add_subdirectory(prep)
