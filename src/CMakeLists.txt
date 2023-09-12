cmake_minimum_required(VERSION 3.21)

project(radix_src)

option(ALP_ENABLE_ADDRESS_SANITIZER "compiles atb with address sanitizer enabled (only debug, works only on g++ and clang)" OFF)
option(ALP_ENABLE_ASSERTS "enable asserts (do not define NDEBUG)" ON)
if (EMSCRIPTEN)
    set(ALP_WWW_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "path to the install directory (for webassembly files, i.e., www directory)")
endif()
if (UNIX AND NOT EMSCRIPTEN AND NOT ANDROID)
    option(ALP_USE_LLVM_LINKER "use lld (llvm) for linking. it's parallel and much faster, but not installed by default.
        if it's not installed, you'll get errors, that openmp or other stuff is not installed (hard to track down)" OFF)
endif()

include(FetchContent)
if(NOT TARGET glm::glm)
    FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git)
    FetchContent_MakeAvailable(glm)
endif()


add_library(radix 
    radix/geometry.h
    radix/hasher.h
    radix/iterator.h
    radix/quad_tree.h
    radix/tile.h
    radix/TileHeights.h radix/TileHeights.cpp
    radix/height_encoding.h)
target_include_directories(radix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(radix PUBLIC glm::glm)
target_compile_features(radix PUBLIC cxx_std_20)
target_compile_definitions(radix PUBLIC GLM_FORCE_XYZW_ONLY GLM_ENABLE_EXPERIMENTAL)

if (ALP_ENABLE_ASSERTS)
    target_compile_options(radix PUBLIC "-U NDEBUG")
    target_compile_definitions(radix PUBLIC TNTN_ASSERT_ACTIVE)
endif()

if (ALP_ENABLE_ADDRESS_SANITIZER)
    message(NOTICE "building with address sanitizer enabled")
    target_compile_options(radix PUBLIC "-fno-omit-frame-pointer -fsanitize=address")
    target_link_options(radix PUBLIC "-fno-omit-frame-pointer -fsanitize=address")
endif()

if (ALP_USE_LLVM_LINKER)
    target_link_options(radix PUBLIC "-fuse-ld=lld")
endif()
