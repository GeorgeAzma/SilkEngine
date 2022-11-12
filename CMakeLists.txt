cmake_minimum_required(VERSION 3.20)

include(FetchContent)
include(ExternalProject)

set(MSVC_INCREMENTAL_DEFAULT ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Get rid of console window in release build
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

project(SilkEngine VERSION 1.0.0 LANGUAGES CXX)

add_compile_definitions(
_USE_MATH_DEFINES
GLFW_INCLUDE_NONE
)

#|-----Vendor-----|#

# Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
list(APPEND INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
list(APPEND EXTRA_LIBS ${Vulkan_LIBRARIES})

string(FIND ${Vulkan_LIBRARIES} "/Lib" Vulkan_PATH_POS)
string(SUBSTRING ${Vulkan_LIBRARIES} 0 ${Vulkan_PATH_POS} VULKAN_PATH)
list(APPEND EXTRA_LIBS 
    ${VULKAN_PATH}/Lib/${CMAKE_STATIC_LIBRARY_PREFIX}shaderc_combined$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
)

macro(fetchContent)
    set(one_value_args NAME GIT_REPO GIT_TAG INCLUDE_DIR)
    cmake_parse_arguments(SUBPROJECT "" "${one_value_args}" "" ${ARGN})
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/content/${SUBPROJECT_NAME}")
        FetchContent_Declare(${SUBPROJECT_NAME}
            GIT_REPOSITORY ${SUBPROJECT_GIT_REPO}
            GIT_TAG ${SUBPROJECT_GIT_TAG}
            GIT_SHALLOW ON
            SOURCE_DIR ${CMAKE_BINARY_DIR}/content/${SUBPROJECT_NAME}
        )
        FetchContent_MakeAvailable(${SUBPROJECT_NAME})
    endif()
    list(APPEND INCLUDE_DIRS ${CMAKE_BINARY_DIR}/content/${SUBPROJECT_NAME}/${SUBPROJECT_INCLUDE_DIR})
endmacro()

macro(externalProject)
    set(one_value_args NAME GIT_REPO GIT_TAG INCLUDE_DIR)
    set(multi_value_args BUILD_ARGS DEPENDS_ON)
    cmake_parse_arguments(SUBPROJECT "" "${one_value_args}" "${multi_value_args}" ${ARGN})
    ExternalProject_Add(${SUBPROJECT_NAME}
        PREFIX ${SUBPROJECT_NAME}
        DOWNLOAD_DIR ${SUBPROJECT_NAME}
        STAMP_DIR ${SUBPROJECT_NAME}/stamp
        SOURCE_DIR ${SUBPROJECT_NAME}/src
        BINARY_DIR ${SUBPROJECT_NAME}/build
        GIT_REPOSITORY ${SUBPROJECT_GIT_REPO}
        GIT_TAG ${SUBPROJECT_GIT_TAG}
        GIT_SHALLOW ON
        CMAKE_ARGS 
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
            -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
            -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
            -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            ${SUBPROJECT_BUILD_ARGS}
        DEPENDS ${SUBPROJECT_DEPENDS_ON}
        BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
        BUILD_ALWAYS OFF
        UPDATE_COMMAND ""
    )
    if(SUBPROJECT_DEPENDS_ON)
        ExternalProject_Add_StepDependencies(${SUBPROJECT_NAME} 
            build ${SUBPROJECT_DEPENDS_ON}
        )
    endif()
    list(APPEND INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include/${SUBPROJECT_INCLUDE_DIR})
    list(APPEND DEPENDENCIES_TO_ADD ${SUBPROJECT_NAME})
endmacro()

# FetchContents
fetchContent(NAME spdlog GIT_REPO https://github.com/gabime/spdlog GIT_TAG v1.x INCLUDE_DIR include)
fetchContent(NAME glm GIT_REPO https://github.com/g-truc/glm GIT_TAG master)
fetchContent(NAME entt GIT_REPO https://github.com/skypjack/entt GIT_TAG master INCLUDE_DIR src)
fetchContent(NAME stb GIT_REPO https://github.com/nothings/stb GIT_TAG master)
fetchContent(NAME vma GIT_REPO https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG master INCLUDE_DIR include)

# External Projects
externalProject(NAME freetype 
    GIT_REPO https://github.com/freetype/freetype 
    GIT_TAG master 
    INCLUDE_DIR freetype2
)
externalProject(NAME glfw 
    GIT_REPO https://github.com/glfw/glfw 
    GIT_TAG master 
    BUILD_ARGS 
        -DGLFW_BUILD_EXAMPLES=OFF
        -DGLFW_BUILD_DOCS=OFF
        -DGLFW_BUILD_TESTS=OFF
)
externalProject(NAME assimp 
    GIT_REPO https://github.com/assimp/assimp 
    GIT_TAG v5.0.0 
    BUILD_ARGS 
        -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF
        -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=OFF
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
        -DASSIMP_BUILD_TESTS=OFF
        -DASSIMP_BUILD_FBX_IMPORTER=ON
        -DASSIMP_BUILD_OBJ_IMPORTER=ON
        -DASSIMP_BUILD_OBJ_EXPORTER=ON
        -DASSIMP_BUILD_ZLIB=ON
        -DBUILD_SHARED_LIBS=OFF
)
externalProject(NAME openal 
    GIT_REPO https://github.com/kcat/openal-soft 
    GIT_TAG master 
    BUILD_ARGS 
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
externalProject(NAME spirv_cross 
    GIT_REPO https://github.com/KhronosGroup/SPIRV-Cross 
    GIT_TAG master 
    BUILD_ARGS 
        -DSPIRV_CROSS_CLI=OFF
        -DSPIRV_CROSS_ENABLE_TESTS=OFF
        -DSPIRV_CROSS_HLSL=OFF
        -DSPIRV_CROSS_MSL=OFF
)
#externalProject(NAME spirv_headers 
#    GIT_REPO https://github.com/KhronosGroup/SPIRV-Headers 
#    GIT_TAG master 
#    BUILD_ARGS 
#        -DSPIRV_HEADERS_SKIP_EXAMPLES=ON
#)
#externalProject(NAME spirv_tools 
#    GIT_REPO https://github.com/KhronosGroup/SPIRV-Tools 
#    GIT_TAG master
#    BUILD_ARGS
#        -DSPIRV_SKIP_EXECUTABLES=ON
#    DEPENDS_ON spirv_headers
#)
#externalProject(NAME shaderc 
#    GIT_REPO https://github.com/google/shaderc 
#    GIT_TAG main 
#    BUILD_ARGS
#        -DSHADERC_SKIP_TESTS=ON
#        -DSHADERC_SKIP_INSTALL=ON 
#        -DSHADERC_SKIP_EXAMPLES=ON 
#    DEPENDS_ON spirv_tools
#)

file(GLOB_RECURSE EXTERNAL_PROJECT_LIBS "${CMAKE_INSTALL_PREFIX}/*${CMAKE_STATIC_LIBRARY_SUFFIX}")
list(APPEND EXTRA_LIBS ${EXTERNAL_PROJECT_LIBS})

# |-----Sub Directories-----|
add_subdirectory("silk_engine")
add_subdirectory("projects")