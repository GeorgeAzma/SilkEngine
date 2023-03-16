cmake_minimum_required(VERSION 3.24)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/CMake" CACHE STRING "Modules for CMake" FORCE)

include(FetchContent)
include(ExternalProject)

set(MSVC_INCREMENTAL_DEFAULT ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_PREFIX}/doc)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(Project LANGUAGES C CXX)

if($<CONFIG:RELEASE>)
    set(CONFIG_MODE RELEASE)
	set(BUILD_TESTS OFF)
else()
    set(CONFIG_MODE DEBUG)
	set(BUILD_TESTS ON)
endif()


# Get rid of console window in release build
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_compile_definitions(
_USE_MATH_DEFINES
GLFW_INCLUDE_NONE
)

#|-----Vendor-----|#

# Vulkan
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)
list(APPEND EXTRA_LIBS ${Vulkan_LIBRARIES} Vulkan::shaderc_combined)

include(BuildExternalProject)

# External Projects
externalProject(NAME freetype INCLUDE_DIR freetype2)
externalProject(NAME glfw BUILD_ARGS
    -DGLFW_BUILD_EXAMPLES=OFF
    -DGLFW_BUILD_DOCS=OFF
    -DGLFW_BUILD_TESTS=OFF
)
externalProject(NAME assimp BUILD_ARGS 
    -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF
    -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=OFF
    -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
    -DASSIMP_BUILD_TESTS=OFF
    -DASSIMP_NO_EXPORT=ON
    -DASSIMP_BUILD_FBX_IMPORTER=ON
    -DASSIMP_BUILD_OBJ_IMPORTER=ON
    -DASSIMP_BUILD_glTF_IMPORTER=ON
    -DASSIMP_BUILD_glTF2_IMPORTER=ON
    -DASSIMP_BUILD_ZLIB=ON
    -DBUILD_SHARED_LIBS=OFF
)
externalProject(NAME openal BUILD_ARGS
    -DALSOFT_INSTALL_EXAMPLES=OFF
    -DALSOFT_INSTALL_AMBDEC_PRESETS=OFF
    -DALSOFT_INSTALL_CONFIG=OFF
    -DALSOFT_INSTALL_UTILS=OFF
    -DALSOFT_INSTALL_HRTF_DATA=OFF
    -DALSOFT_EXAMPLES=OFF
    -DALSOFT_UTILS=OFF
    -DALSOFT_NO_CONFIG_UTIL=ON
    -DLIBTYPE=STATIC
)
externalProject(NAME spirv_cross BUILD_ARGS 
    -DSPIRV_CROSS_CLI=OFF
    -DSPIRV_CROSS_ENABLE_GLSL=OFF
    -DSPIRV_CROSS_ENABLE_HLSL=OFF
    -DSPIRV_CROSS_ENABLE_TESTS=OFF
    -DSPIRV_CROSS_ENABLE_MSL=OFF
    -DSPIRV_CROSS_ENABLE_CPP=OFF
    -DSPIRV_CROSS_ENABLE_REFLECT=OFF
    -DSPIRV_CROSS_ENABLE_UTIL=OFF
)

macro(subdirList result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
      if(IS_DIRECTORY ${curdir}/${child})
        list(APPEND dirlist ${child})
      endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

list(APPEND INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)

subdirList(INCLUDE_SUBDIRS ${CMAKE_SOURCE_DIR}/ext)
if(INCLUDE_SUBDIRS)
    foreach(subdir ${INCLUDE_SUBDIRS})
        list(APPEND INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/ext/${subdir})
    endforeach()
endif()

file(GLOB_RECURSE EXTERNAL_PROJECT_LIBS "${CMAKE_INSTALL_PREFIX}/*${CMAKE_STATIC_LIBRARY_SUFFIX}")

if(EXTERNAL_PROJECT_LIBS)
    list(APPEND EXTRA_LIBS ${EXTERNAL_PROJECT_LIBS})
endif()

# |-----Sub Directories-----|
add_subdirectory("silk_engine")
add_subdirectory("src")
if(${BUILD_TESTS})
    add_subdirectory("tests")
endif()