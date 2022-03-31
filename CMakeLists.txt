cmake_minimum_required(VERSION 3.20)
set(MSVC_INCREMENTAL_DEFAULT ON)
project(SilkEngine VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
include(ExternalProject)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source build detected!")
endif()

#Some good to have code
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

#spdlog
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/spdlog/include)
list(APPEND FETCHED_CONTENTS SPDLOG)
FetchContent_Declare(SPDLOG
GIT_REPOSITORY  https://github.com/gabime/spdlog
GIT_TAG         v1.9.2
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/spdlog
)
FetchContent_MakeAvailable(SPDLOG)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/spdlog/include)

#glfw
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
list(APPEND DEPENDENCIES_TO_ADD GLFW)
ExternalProject_Add(GLFW
GIT_REPOSITORY  https://github.com/glfw/glfw
GIT_TAG         3.3.5
GIT_SHALLOW     ON
CMAKE_ARGS   -DGLFW_BUILD_EXAMPLES=OFF
             -DGLFW_BUILD_DOCS=OFF
             -DGLFW_BUILD_TESTS=OFF
             -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/glfw
UPDATE_COMMAND ""
)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})

#glm
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/glm/glm)
list(APPEND FETCHED_CONTENTS GLM)
FetchContent_Declare(GLM
GIT_REPOSITORY  https://github.com/g-truc/glm
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/glm
)
FetchContent_MakeAvailable(GLM)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glm)

#entt
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/entt/src)
list(APPEND FETCHED_CONTENTS ENTT)
FetchContent_Declare(ENTT
GIT_REPOSITORY  https://github.com/skypjack/entt
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/entt
)
FetchContent_MakeAvailable(ENTT)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/entt/src)

#stb
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/stb)
list(APPEND FETCHED_CONTENTS STB)
FetchContent_Declare(STB
GIT_REPOSITORY   https://github.com/nothings/stb
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/stb
)
FetchContent_MakeAvailable(STB)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/stb)

#FreeType
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/freetype/lib/${CMAKE_STATIC_LIBRARY_PREFIX}freetype$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX})
list(APPEND DEPENDENCIES_TO_ADD FREETYPE)
ExternalProject_Add(FREETYPE
GIT_REPOSITORY   https://github.com/freetype/freetype
GIT_TAG origin/master
GIT_SHALLOW     ON
CMAKE_ARGS   -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/freetype
UPDATE_COMMAND ""
)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/freetype/include/freetype2)
list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/freetype/lib/${CMAKE_STATIC_LIBRARY_PREFIX}freetype$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX})

#vma
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/vma/include)
list(APPEND FETCHED_CONTENTS VMA)
FetchContent_Declare(VMA
GIT_REPOSITORY   https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/vma
)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/vma/include)

#assimp
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}assimp$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX})
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
UPDATE_COMMAND ""
)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/assimp/include)
list(APPEND EXTRA_LIBS 
${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}assimp$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
${PROJECT_SOURCE_DIR}/vendor/assimp/lib/${CMAKE_STATIC_LIBRARY_PREFIX}irrXML$<$<CONFIG:DEBUG>:d>${CMAKE_STATIC_LIBRARY_SUFFIX}
)

FetchContent_MakeAvailable(${FETCHED_CONTENTS})

#Sub dirs
add_subdirectory("silk_engine/src")
add_subdirectory("sandbox/src")