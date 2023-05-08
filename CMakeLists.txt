cmake_minimum_required(VERSION 3.21)

project(sherpa)

option(ALP_UNITTESTS "include unit test targets in the buildsystem" ON)
option(ALP_ENABLE_ADDRESS_SANITIZER "compiles atb with address sanitizer enabled (only debug, works only on g++ and clang)" OFF)
option(ALP_ENABLE_ASSERTS "enable asserts (do not define NDEBUG)" ON)
set(ALP_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "path to the install directory (for webassembly files, i.e., www directory)")

if (UNIX AND NOT EMSCRIPTEN AND NOT ANDROID)
    option(ALP_USE_LLVM_LINKER "use lld (llvm) for linking. it's parallel and much faster, but not installed by default.
        if it's not installed, you'll get errors, that openmp or other stuff is not installed (hard to track down)" OFF)
endif()

set(CMAKE_CXX_STANDARD 20)

set(SHERPA_SOURCES
    geometry.h
    hasher.h
    iterator.h
    quad_tree.h
    tile.h
    TileHeights.h TileHeights.cpp
)

set(SHERPA_UNITTESTS_SOURCES
    unittests/geometry.cpp
    unittests/iterator.cpp
    unittests/main.cpp
    unittests/quad_tree.cpp
    unittests/tile.cpp
    unittests/tile_heights.cpp
)

include(FetchContent)
if(NOT TARGET glm::glm)
    FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git)
    FetchContent_MakeAvailable(glm)
endif()
if(ALP_UNITTESTS OR BUILD_TESTING)
    FetchContent_Declare(catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.3.2)
    FetchContent_MakeAvailable(catch2)
endif()

if (ALP_ENABLE_ADDRESS_SANITIZER)
    message(NOTICE "building with address sanitizer enabled")
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

if (ALP_USE_LLVM_LINKER)
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -fuse-ld=lld")
endif()


add_library(sherpa STATIC ${SHERPA_SOURCES})

target_include_directories(sherpa
    PRIVATE .
)
target_link_libraries(sherpa PUBLIC glm::glm)
target_compile_definitions(sherpa PUBLIC GLM_FORCE_XYZW_ONLY GLM_ENABLE_EXPERIMENTAL)
if (ALP_ENABLE_ASSERTS)
    target_compile_options(sherpa PUBLIC "-U NDEBUG")
    target_compile_options(sherpa PUBLIC "-DTNTN_ASSERT_ACTIVE")
endif()

if (ALP_UNITTESTS)
    add_executable(sherpa_unittests ${SHERPA_UNITTESTS_SOURCES})
    target_link_libraries(sherpa_unittests PUBLIC sherpa Catch2::Catch2WithMain)
    if (EMSCRIPTEN)
        set_target_properties(sherpa_unittests PROPERTIES SUFFIX ".html")
        set(ALP_INSTALL_FILES
            "$<TARGET_FILE_DIR:sherpa_unittests>/sherpa_unittests.js"
            "$<TARGET_FILE_DIR:sherpa_unittests>/sherpa_unittests.wasm"
            "$<TARGET_FILE_DIR:sherpa_unittests>/sherpa_unittests.html"
        )
        install(FILES ${ALP_INSTALL_FILES} DESTINATION ${ALP_INSTALL_DIR})
    endif()
    target_include_directories(sherpa_unittests
        PRIVATE .
    )
endif()
