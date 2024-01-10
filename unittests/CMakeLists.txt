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

project(radix_unittests LANGUAGES CXX)

if (NOT TARGET Catch2)
    alp_add_git_repository(catch2 URL https://github.com/catchorg/Catch2.git COMMITISH v3.4.0)
endif()

set(RADIX_UNITTESTS_SOURCES
    geometry.cpp
    iterator.cpp
    main.cpp
    quad_tree.cpp
    tile.cpp
    tile_heights.cpp
    height_encoding.cpp
)
if (ANDROID)
    qt_add_executable(unittests_radix ${RADIX_UNITTESTS_SOURCES})
    target_link_libraries(unittests_radix PUBLIC Qt::Core Qt::Gui)
    install(TARGETS unittests_radix
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    add_executable(unittests_radix ${RADIX_UNITTESTS_SOURCES})
endif()
target_link_libraries(unittests_radix PUBLIC radix Catch2::Catch2)

if (EMSCRIPTEN)
    set_target_properties(unittests_radix PROPERTIES SUFFIX ".html")
    target_compile_options(unittests_radix PUBLIC -sASSERTIONS -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=104857600)
    set(ALP_INSTALL_FILES
        "$<TARGET_FILE_DIR:unittests_radix>/unittests_radix.js"
        "$<TARGET_FILE_DIR:unittests_radix>/unittests_radix.wasm"
        "$<TARGET_FILE_DIR:unittests_radix>/unittests_radix.html"
    )
    install(FILES ${ALP_INSTALL_FILES} DESTINATION ${ALP_WWW_INSTALL_DIR})
endif()
