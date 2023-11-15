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

cmake_minimum_required(VERSION 3.24)
project(radix)

option(ALP_UNITTESTS "include unit test targets in the buildsystem" ON)
set(ALP_EXTERN_DIR "3rdparty" CACHE STRING "name of the directory to store external libraries, fonts etc..")

include(cmake/alp_add_git_repository.cmake)

add_subdirectory(src)
if (ALP_UNITTESTS)
    add_subdirectory(unittests)
endif()
