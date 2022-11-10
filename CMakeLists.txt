cmake_minimum_required(VERSION 3.18)

project(sherpa)

option(SHERPA_UNITTESTS "include unit test targets in the buildsystem" ON)
option(SHERPA_ENABLE_ADDRESS_SANITIZER "compiles atb with address sanitizer enabled (only debug, works only on g++ and clang)" ON)
option(SHERPA_ENABLE_ASSERTS "enable asserts (do not define NDEBUG)" ON)
option(SHERPA_USE_LLVM_LINKER "use lld (llvm) for linking. it's parallel and much faster, but not installed by default. if it's not installed, you'll get errors, that openmp or other stuff is not installed (hard to track down)" OFF)

set(CMAKE_CXX_STANDARD 20)

if (SHERPA_UNITTESTS AND NOT EMSCRIPTEN)
    find_package(Catch2 REQUIRED)
endif()

set(SHERPA_SOURCES
    geometry.h
    hasher.h
    tile.h
    TileHeights.h TileHeights.cpp
)

set(SHERPA_UNITTESTS_SOURCES
    unittests/geometry.cpp
    unittests/main.cpp
    unittests/tile.cpp
    unittests/tile_heights.cpp
)


if (SHERPA_ENABLE_ADDRESS_SANITIZER)
    message(NOTICE "building with address sanitizer enabled")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

if (SHERPA_USE_LLVM_LINKER)
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld")
endif()


add_library(sherpa STATIC ${SHERPA_SOURCES})

target_include_directories(sherpa SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/libs/glm)
target_compile_definitions(sherpa PUBLIC GLM_FORCE_XYZW_ONLY)
target_include_directories(sherpa PRIVATE .)
if (SHERPA_ENABLE_ASSERTS)
    target_compile_options(sherpa PUBLIC "-U NDEBUG")
    target_compile_options(sherpa PUBLIC "-DTNTN_ASSERT_ACTIVE")
endif()

if (SHERPA_UNITTESTS AND NOT EMSCRIPTEN)
    add_executable(sherpa_unittests ${SHERPA_UNITTESTS_SOURCES})
    target_link_libraries(sherpa_unittests PUBLIC sherpa Catch2::Catch2)
endif()
