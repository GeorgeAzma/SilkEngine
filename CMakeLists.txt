cmake_minimum_required(VERSION 3.24)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE STRING "Modules for CMake" FORCE)

include(Config)
include(Utils)

project(Project LANGUAGES C CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(IncludeExternalDependencies)

# |-----Sub Directories-----|
add_subdirectory("silk_engine")
add_subdirectory("src")
if(${BUILD_TESTS})
    add_subdirectory("tests")
endif()