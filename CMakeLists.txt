cmake_minimum_required(VERSION 3.21)

project(radix_project)

option(ALP_UNITTESTS "include unit test targets in the buildsystem" ON)

add_subdirectory(src)
if (ALP_UNITTESTS)
    add_subdirectory(unittests)
endif()
