cmake_minimum_required(VERSION 3.20)
project(fcc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LLVM_DIR "C:/msys64/mingw64/lib/cmake/llvm")
set(ZLIB_ROOT "C:/msys64/mingw64")

find_package(ZLIB REQUIRED)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(src)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(fcc ${SOURCES} ${HEADERS}
        src/Debug.cpp
        src/Debug.h)

llvm_map_components_to_libnames(llvm_libs
        Core
        Support
        Target
        X86CodeGen
        X86AsmParser
        X86Info
)

target_link_libraries(fcc ${llvm_libs} ZLIB::ZLIB)