cmake_minimum_required(VERSION 3.24)
project(radix)

option(ALP_UNITTESTS "include unit test targets in the buildsystem" ON)
set(ALP_EXTERN_DIR "3rdparty" CACHE STRING "name of the directory to store external libraries, fonts etc..")

add_subdirectory(src)
if (ALP_UNITTESTS)
    add_subdirectory(unittests)
endif()
