cmake_minimum_required(VERSION 3.20)
project(VulkanEngine VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
include(ExternalProject)

#Some good to have code
set(CMAKE_CXX_STANDARD 20)

if(MSVC)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_compile_definitions(
GLEW_STATIC 
_USE_MATH_DEFINES
GLM_FORCE_RADIANS
GLM_FORCE_DEPTH_ZERO_TO_ONE
)

#|-----Vendor-----|#

#Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
list(APPEND INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
list(APPEND EXTRA_LIBS ${Vulkan_LIBRARIES})

#thread_pool
FetchContent_Declare(THREAD_POOL
GIT_REPOSITORY  https://github.com/bshoshany/thread-pool
GIT_TAG         v2.0.0
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/thread_pool
)
FetchContent_MakeAvailable(THREAD_POOL)
FetchContent_GetProperties(THREAD_POOL)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/thread_pool)

#spdlog
FetchContent_Declare(SPDLOG
GIT_REPOSITORY  https://github.com/gabime/spdlog
GIT_TAG         v1.9.2
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/spdlog
)
FetchContent_MakeAvailable(SPDLOG)
FetchContent_GetProperties(SPDLOG)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/spdlog/include)

find_package(Threads REQUIRED)

#glfw
set(glfw_build FALSE)
list(APPEND DEPENDENCIES_TO_ADD glfw_build)
ExternalProject_Add(GLFW
GIT_REPOSITORY  https://github.com/glfw/glfw
GIT_TAG         3.3.5
GIT_SHALLOW     ON
INSTALL_DIR ${PROJECT_SOURCE_DIR}/vendor/glfw
CMAKE_ARGS   -DGLFW_BUILD_EXAMPLES=OFF
             -DGLFW_BUILD_TESTS=OFF
             -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/glfw
)
set(glfw_build TRUE)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
if(WIN32)
    list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/glfw3.lib)
else()
    list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/libglfw3.a)
endif()

#glm
FetchContent_Declare(GLM
GIT_REPOSITORY  https://github.com/g-truc/glm
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/glm
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(GLM)
FetchContent_GetProperties(GLM)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glm)

#entt
FetchContent_Declare(ENTT
GIT_REPOSITORY  https://github.com/skypjack/entt
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/entt
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(ENTT)
FetchContent_GetProperties(ENTT)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/entt)

#stb
FetchContent_Declare(STB
GIT_REPOSITORY   https://github.com/nothings/stb
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/stb
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
)
FetchContent_MakeAvailable(STB)
FetchContent_GetProperties(STB)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/stb)

#assimp
set(assimp_build FALSE)
list(APPEND DEPENDENCIES_TO_ADD assimp_build)
ExternalProject_Add(ASSIMP
GIT_REPOSITORY  https://github.com/assimp/assimp
GIT_TAG         v5.0.0
GIT_SHALLOW     ON
UPDATE_COMMAND  ""
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
                -DCMAKE_DEBUG_POSTFIX:STRING=
                -DASSIMP_INJECT_DEBUG_POSTFIX:BOOL=OFF
                -DBUILD_SHARED_LIBS:BOOL=OFF
                -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
)
set(assimp_build TRUE)
list(APPEND INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/vendor/assimp/include")
if(MSVC)
list(APPEND EXTRA_LIBS "assimp" "$<IF:$<CONFIG:Debug>,zlibstaticd,zlibstatic>" "irrXML")
else()
	list(APPEND EXTRA_LIBS assimp zlibstatic irrXML)
endif()
link_directories("${PROJECT_SOURCE_DIR}/vendor/assimp/lib")

#Sub dirs
############################__VULKAN ENGINE__#####################################
file(GLOB_RECURSE VULKAN_ENGINE_SOURCES 
"vulkan_engine/src/*.cpp" 
"vulkan_engine/src/*.h" 
"vulkan_engine/src/*.hpp")

#Create VulkanEngine.lib
add_library(VulkanEngine STATIC ${VULKAN_ENGINE_SOURCES} "vulkan_engine/src/utils/math.h")

#Include dirs
target_include_directories(VulkanEngine PUBLIC 
"${PROJECT_SOURCE_DIR}/vendor" 
"${PROJECT_SOURCE_DIR}/vulkan_engine/src"
${INCLUDE_DIRS}
)

#Add dependencies
foreach(item IN LISTS DEPENDENCIES_TO_ADD)

    if("${${item}}" MATCHES TRUE)
        string(FIND ${item} "_" UNDERLINE_POS)
        string(SUBSTRING ${item} 0 ${UNDERLINE_POS} USABLE_STRING)
        string(TOUPPER ${USABLE_STRING} UPPER_STRING)
        add_dependencies(VulkanEngine ${UPPER_STRING})
    endif()

endforeach()

#Defines
target_compile_definitions(VulkanEngine PUBLIC 
"$<$<CONFIG:DEBUG>:VE_DEBUG>"
"$<$<CONFIG:RELEASE>:VE_DIST>"
)
target_compile_definitions(VulkanEngine PRIVATE 
"VE_CORE"
)

#Link
target_link_directories(VulkanEngine PUBLIC ${EXTRA_LIB_DIRS})
target_link_libraries(VulkanEngine ${EXTRA_LIBS})

#Precompile headers
target_precompile_headers(VulkanEngine PUBLIC 
vulkan_engine/src/core/base.h
<iostream>
<string>
<unordered_map>
<unordered_set>
<map>
<set>
<vector>
<chrono>
<sstream>
<fstream>
<memory>
<functional>
<algorithm>
<utility>
<array>
<random>
<stdlib.h>
<thread>
<queue>
<mutex>
<condition_variable>
<bit>
<math.h>
<filesystem>
<future>
<stdexcept>
<cstdlib>
<numeric>
<optional>
<typeindex>
<typeinfo>
<list>
<GLM/glm.hpp>
<GLM/gtc/matrix_transform.hpp>
<GLM/gtx/quaternion.hpp>
<vulkan/vulkan.h>
<GLFW/glfw3.h>
vulkan_engine/src/core/log.h)


############################__SANDBOX__#####################################
file(GLOB_RECURSE SANDBOX_SOURCES "sandbox/src/*.cpp" "sandbox/src/*.h" "sandbox/src/*.hpp")

add_executable(Sandbox ${SANDBOX_SOURCES} "vulkan_engine/src/utils/math.h")

target_link_libraries(Sandbox PRIVATE VulkanEngine)

#Shader compilation
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin
  $ENV{VULKAN_SDK}/Bin32
)
 
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/data/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/data/shaders/*.frag"
)
 
foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/data/cache/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})

add_dependencies(Sandbox Shaders)

#Copy data folder where exe file is
add_custom_command(TARGET Sandbox POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory 
${PROJECT_SOURCE_DIR}/data 
$<TARGET_FILE_DIR:Sandbox>/data)