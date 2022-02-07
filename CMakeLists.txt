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
GLM_FORCE_LEFT_HANDED
)

#|-----Vendor-----|#

#Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
list(APPEND INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
list(APPEND EXTRA_LIBS ${Vulkan_LIBRARIES})
string(FIND ${Vulkan_LIBRARIES} "/Lib" Vulkan_PATH_POS)
string(SUBSTRING ${Vulkan_LIBRARIES} 0 ${Vulkan_PATH_POS} VULKAN_PATH)
list(APPEND EXTRA_LIBS 
    ${VULKAN_PATH}/Lib/${CMAKE_STATIC_LIBRARY_PREFIX}shaderc_combined${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${VULKAN_PATH}/Lib/${CMAKE_STATIC_LIBRARY_PREFIX}spirv-cross-core${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${VULKAN_PATH}/Lib/${CMAKE_STATIC_LIBRARY_PREFIX}spirv-cross-cpp${CMAKE_STATIC_LIBRARY_SUFFIX}
)

#spdlog
FetchContent_Declare(SPDLOG
GIT_REPOSITORY  https://github.com/gabime/spdlog
GIT_TAG         v1.9.2
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/spdlog
UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(SPDLOG)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/spdlog/include)

find_package(Threads REQUIRED)

#glfw
list(APPEND DEPENDENCIES_TO_ADD GLFW)
ExternalProject_Add(GLFW
GIT_REPOSITORY  https://github.com/glfw/glfw
GIT_TAG         3.3.5
GIT_SHALLOW     ON
CMAKE_ARGS   -DGLFW_BUILD_EXAMPLES=OFF
             -DGLFW_BUILD_TESTS=OFF
             -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/glfw
UPDATE_COMMAND ""
)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})

#glm
FetchContent_Declare(GLM
GIT_REPOSITORY  https://github.com/g-truc/glm
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/glm
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(GLM)
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
UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(ENTT)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/entt/src)

#stb
FetchContent_Declare(STB
GIT_REPOSITORY   https://github.com/nothings/stb
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/stb
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(STB)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/stb)

#FreeType
list(APPEND DEPENDENCIES_TO_ADD FREETYPE)
ExternalProject_Add(FREETYPE
GIT_REPOSITORY   https://github.com/freetype/freetype
GIT_TAG origin/master
GIT_SHALLOW     ON
CMAKE_ARGS   -DCMAKE_INSTALL_PREFIX=${PROJECT_SOURCE_DIR}/vendor/freetype
UPDATE_COMMAND ""
)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/freetype/include/freetype2)
list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/freetype/lib/${CMAKE_STATIC_LIBRARY_PREFIX}$<IF:$<CONFIG:DEBUG>,freetyped,freetype>${CMAKE_STATIC_LIBRARY_SUFFIX})

#vma
FetchContent_Declare(VMA
GIT_REPOSITORY   https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/vma
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
UPDATE_COMMAND  ""
)
FetchContent_MakeAvailable(VMA)
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/vma/include)

#assimp
list(APPEND DEPENDENCIES_TO_ADD ASSIMP)
ExternalProject_Add(ASSIMP
GIT_REPOSITORY  https://github.com/assimp/assimp
GIT_TAG         v5.0.0
GIT_SHALLOW     ON
UPDATE_COMMAND  ""
CMAKE_ARGS      -DASSIMP_LIB_INSTALL_DIR=${PROJECT_BINARY_DIR}/vendor/assimp/lib
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
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/assimp/include)
list(APPEND EXTRA_LIBS "assimp" "$<IF:$<CONFIG:Debug>,zlibstaticd,zlibstatic>" "irrXML")

link_directories("${PROJECT_BINARY_DIR}/vendor/assimp/lib")

#Sub dirs
############################__SILK ENGINE__#####################################
file(GLOB_RECURSE SILK_ENGINE_SOURCES 
"silk_engine/src/*.cpp")

#Create SilkEngine.lib
add_library(SilkEngine STATIC ${SILK_ENGINE_SOURCES})

#Include dirs
target_include_directories(SilkEngine PUBLIC 
"${PROJECT_SOURCE_DIR}/vendor" 
"${PROJECT_SOURCE_DIR}/silk_engine/src"
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
target_compile_definitions(SilkEngine PUBLIC 
"$<$<CONFIG:DEBUG>:SK_DEBUG>"
"$<$<CONFIG:RELEASE>:SK_DIST>"
)
target_compile_definitions(SilkEngine PRIVATE 
"SK_CORE"
)

#Precompile headers
target_precompile_headers(SilkEngine PUBLIC 
silk_engine/src/core/base.h
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
<entt/entt.hpp>
<semaphore>
<thread>
<concepts>
silk_engine/src/core/log.h
silk_engine/src/io/file.h
silk_engine/src/utils/math.h
silk_engine/src/utils/RNG.h
silk_engine/src/utils/thread_pool.h)

#Link
target_link_directories(SilkEngine PUBLIC ${EXTRA_LIB_DIRS})
target_link_libraries(SilkEngine ${EXTRA_LIBS})

############################__SANDBOX__#####################################
file(GLOB_RECURSE SANDBOX_SOURCES "sandbox/src/*.cpp")

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox PRIVATE SilkEngine)

#Copy data folder where exe file is
add_custom_command(TARGET Sandbox POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory 
${PROJECT_SOURCE_DIR}/data 
$<TARGET_FILE_DIR:Sandbox>/data)