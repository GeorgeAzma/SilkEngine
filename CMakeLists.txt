cmake_minimum_required(VERSION 3.20)
set(MSVC_INCREMENTAL_DEFAULT ON)
project(SilkEngine VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
include(ExternalProject)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source build detected!")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX d)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_compile_definitions(
GLEW_STATIC 
_USE_MATH_DEFINES
GLM_FORCE_RADIANS
GLM_FORCE_DEPTH_ZERO_TO_ONE
GLM_FORCE_LEFT_HANDED
GLM_ENABLE_EXPERIMENTAL
GLFW_INCLUDE_NONE
)

#|-----Vendor-----|#

#Vulkan
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
    list(APPEND FETCHED_CONTENTS ${project_name})
    FetchContent_Declare(${project_name}
    GIT_REPOSITORY  ${git_repo}
    GIT_TAG         ${git_tag}
    GIT_SHALLOW     ON
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/${project_name}
    )
    list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/${project_name}/${source_dir})
endmacro()

#FetchContents

fetchContent(SPDLOG https://github.com/gabime/spdlog v1.9.2 include)
fetchContent(GLM https://github.com/g-truc/glm origin/master "")
fetchContent(ENTT https://github.com/skypjack/entt origin/master src)
fetchContent(STB https://github.com/nothings/stb origin/master "")
fetchContent(VMA https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator origin/master include)
fetchContent(EIGEN https://github.com/libigl/eigen origin/master "")
FetchContent_MakeAvailable(${FETCHED_CONTENTS})

#ExternalProjects

#glfw
list(APPEND DEPENDENCIES_TO_ADD GLFW)
ExternalProject_Add(GLFW
GIT_REPOSITORY  https://github.com/glfw/glfw
GIT_TAG         origin/master
GIT_SHALLOW     ON
CMAKE_ARGS   -DGLFW_BUILD_EXAMPLES=OFF
             -DGLFW_BUILD_DOCS=OFF
             -DGLFW_BUILD_TESTS=OFF
             -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/glfw
UPDATE_COMMAND  ""
)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})

#freetype
list(APPEND DEPENDENCIES_TO_ADD FREETYPE)
ExternalProject_Add(FREETYPE
GIT_REPOSITORY   https://github.com/freetype/freetype
GIT_TAG origin/master
GIT_SHALLOW     ON
CMAKE_ARGS   -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/freetype
UPDATE_COMMAND  ""
)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/freetype/include/freetype2)
list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/freetype/lib/${CMAKE_STATIC_LIBRARY_PREFIX}freetype$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX})

#assimp
list(APPEND DEPENDENCIES_TO_ADD ASSIMP)
ExternalProject_Add(ASSIMP
GIT_REPOSITORY  https://github.com/assimp/assimp
GIT_TAG         v5.0.0
GIT_SHALLOW     ON
CMAKE_ARGS      -DASSIMP_LIB_INSTALL_DIR=${PROJECT_SOURCE_DIR}/vendor/assimp/lib
                -DASSIMP_INCLUDE_INSTALL_DIR=${PROJECT_SOURCE_DIR}/vendor/assimp/include
                -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT:BOOL=OFF
                -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT:BOOL=OFF
                -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
                -DASSIMP_BUILD_TESTS:BOOL=OFF
                -DASSIMP_BUILD_FBX_IMPORTER:BOOL=ON
                -DASSIMP_BUILD_OBJ_IMPORTER:BOOL=ON
                -DASSIMP_BUILD_OBJ_EXPORTER:BOOL=ON
                -DASSIMP_BUILD_ZLIB:BOOL=ON
                -DASSIMP_LIBRARY_SUFFIX:STRING=
                -DLIBRARY_SUFFIX:STRING=
                -DCMAKE_DEBUG_POSTFIX:STRING=${CMAKE_DEBUG_POSTFIX}
                -DASSIMP_INJECT_DEBUG_POSTFIX:BOOL=OFF
                -DBUILD_SHARED_LIBS:BOOL=OFF
                -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
UPDATE_COMMAND  ""
)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/assimp/include)
list(APPEND EXTRA_LIBS 
${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}assimp$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}irrXML$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
)

#|-----Sub Directories-----|
add_subdirectory("silk_engine/src")
add_subdirectory("sandbox/src")