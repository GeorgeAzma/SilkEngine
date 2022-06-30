cmake_minimum_required(VERSION 3.20)

include(FetchContent)
include(ExternalProject)

set(MSVC_INCREMENTAL_DEFAULT ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Get rid of console window in release build
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

project(SilkEngine VERSION 1.0.0 LANGUAGES CXX)

add_compile_definitions(
_USE_MATH_DEFINES
GLM_FORCE_RADIANS
GLM_FORCE_DEPTH_ZERO_TO_ONE
GLM_FORCE_LEFT_HANDED
GLM_ENABLE_EXPERIMENTAL
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
    ${VULKAN_PATH}/Lib/${CMAKE_STATIC_LIBRARY_PREFIX}spirv-cross-core$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
)

macro(fetchContent project_name git_repo git_tag source_dir)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/content/${project_name}")
        FetchContent_Declare(${project_name}
            GIT_REPOSITORY ${git_repo}
            GIT_TAG ${git_tag}
            GIT_SHALLOW ON
            SOURCE_DIR ${CMAKE_BINARY_DIR}/content/${project_name}
        )
        FetchContent_MakeAvailable(${project_name})
    endif()
    list(APPEND INCLUDE_DIRS ${CMAKE_BINARY_DIR}/content/${project_name}/${source_dir})
endmacro()

macro(externalProject project_name git_repo git_tag include_dir cmake_args)
    ExternalProject_Add(${project_name}
        GIT_REPOSITORY ${git_repo}
        GIT_TAG ${git_tag}
        GIT_SHALLOW ON
        CMAKE_ARGS ${cmake_args} 
                   -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                   -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                   -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
        UPDATE_COMMAND ""
    )
    list(APPEND INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include/${include_dir})
    list(APPEND DEPENDENCIES_TO_ADD ${project_name})
endmacro()

# FetchContents
fetchContent(spdlog https://github.com/gabime/spdlog v1.9.2 include)
fetchContent(glm https://github.com/g-truc/glm origin/master "")
fetchContent(entt https://github.com/skypjack/entt origin/master src)
fetchContent(stb https://github.com/nothings/stb origin/master "")
fetchContent(vma https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator origin/master include)

# ExternalProjects
externalProject(glfw https://github.com/glfw/glfw origin/master "" "-DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF")
externalProject(freetype https://github.com/freetype/freetype origin/master freetype2 "")
externalProject(assimp https://github.com/assimp/assimp "v5.0.0" "" "assimp;zlibstatic;irrXML" "-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT:BOOL=OFF -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT:BOOL=OFF -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF -DASSIMP_BUILD_TESTS:BOOL=OFF -DASSIMP_BUILD_FBX_IMPORTER:BOOL=ON -DASSIMP_BUILD_OBJ_IMPORTER:BOOL=ON -DASSIMP_BUILD_OBJ_EXPORTER:BOOL=ON -DASSIMP_BUILD_ZLIB:BOOL=ON -DASSIMP_LIBRARY_SUFFIX:STRING= -DLIBRARY_SUFFIX:STRING= -DASSIMP_INJECT_DEBUG_POSTFIX:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW")
externalProject(eigen https://gitlab.com/libeigen/eigen origin/master eigen3 "")

file(GLOB EXTERNAL_PROJECT_LIBS "${CMAKE_INSTALL_PREFIX}/lib/*${CMAKE_STATIC_LIBRARY_SUFFIX}")
list(APPEND EXTRA_LIBS ${EXTERNAL_PROJECT_LIBS})

# |-----Sub Directories-----|
add_subdirectory("silk_engine/src")
add_subdirectory("sandbox/src")