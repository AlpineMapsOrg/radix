#############################################################################
# Alpine Radix
# Copyright (C) 2023 Adam Celarek <family name at cg tuwien ac at>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#############################################################################

cmake_minimum_required(VERSION 3.25)
project(radix_src LANGUAGES CXX)

option(ALP_ENABLE_ADDRESS_SANITIZER "compiles atb with address sanitizer enabled (only debug, works only on g++ and clang)" OFF)
option(ALP_ENABLE_ASSERTS "enable asserts (do not define NDEBUG)" ON)
if (EMSCRIPTEN)
    set(ALP_WWW_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH "path to the install directory (for webassembly files, i.e., www directory)")
endif()
if (UNIX AND NOT EMSCRIPTEN AND NOT ANDROID)
    option(ALP_USE_LLVM_LINKER "use lld (llvm) for linking. it's parallel and much faster, but not installed by default.
        if it's not installed, you'll get errors, that openmp or other stuff is not installed (hard to track down)" OFF)
endif()

if (NOT TARGET glm::glm)
    alp_add_git_repository(glm URL https://github.com/g-truc/glm.git COMMITISH b06b775c1c80af51a1183c0e167f9de3b2351a79)
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
    target_compile_options(radix PUBLIC -UNDEBUG)
endif()

if (ALP_ENABLE_ADDRESS_SANITIZER)
    message(NOTICE "building with address sanitizer enabled")
    if(MSVC)
        target_compile_options (radix PUBLIC /fsanitize=address)
    else()
        target_compile_options(radix PUBLIC -fno-omit-frame-pointer -fsanitize=address)
        target_link_options(radix PUBLIC -fno-omit-frame-pointer -fsanitize=address)
    endif()
endif()

if (MSVC)
    target_compile_options(radix PUBLIC /W4 #[[/WX]])
else()
    target_compile_options(radix PUBLIC -Wall -Wextra -pedantic#[[ -Werror]])
endif()

if (ALP_USE_LLVM_LINKER)
    target_link_options(radix PUBLIC "-fuse-ld=lld")
endif()
