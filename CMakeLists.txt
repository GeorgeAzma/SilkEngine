cmake_minimum_required(VERSION 3.20)
project(VulkanEngine VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
include(ExternalProject)

#Some good to have code
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_definitions(GLEW_STATIC)
add_compile_definitions(_USE_MATH_DEFINES)

#|-----Vendor-----|#

#Vulkan
find_package(Vulkan)
if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
	set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
	STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()
list(APPEND INCLUDE_DIRS "${VULKAN_PATH}/Include")
list(APPEND EXTRA_LIB_DIRS "${VULKAN_PATH}/Lib")

#thread_pool
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/thread_pool/.github)
FetchContent_Declare(THREAD_POOL
GIT_REPOSITORY  https://github.com/bshoshany/thread-pool
GIT_TAG         v2.0.0
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/thread_pool
)
FetchContent_MakeAvailable(THREAD_POOL)
FetchContent_GetProperties(THREAD_POOL)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/thread_pool)

#spdlog
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/spdlog/.github)
FetchContent_Declare(SPDLOG
GIT_REPOSITORY  https://github.com/gabime/spdlog
GIT_TAG         v1.9.2
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/spdlog
)
FetchContent_MakeAvailable(SPDLOG)
FetchContent_GetProperties(SPDLOG)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/spdlog/include)

find_package(Threads REQUIRED)

#glfw
set(glfw_build FALSE)
list(APPEND DEPENDENCIES_TO_ADD glfw_build)
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
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
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glfw/include)
if(WIN32)
    list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/glfw3.lib)
else()
    list(APPEND EXTRA_LIBS ${PROJECT_SOURCE_DIR}/vendor/glfw/lib/libglfw3.a)
endif()

#glm
set(glm_deps FALSE)
list(APPEND DEPENDENCIES_TO_ADD glm_deps)
if(NOT EXISTS ${PROJECT_SOURCE_DIR}/vendor/glm/cmake)
ExternalProject_Add(GLM
GIT_REPOSITORY  https://github.com/g-truc/glm
GIT_TAG origin/master
GIT_SHALLOW     ON
SOURCE_DIR ${PROJECT_SOURCE_DIR}/vendor/glm
BUILD_COMMAND ""
CONFIGURE_COMMAND ""
INSTALL_COMMAND ""
)
endif()
list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/vendor/glm)

#Sub dirs
add_subdirectory("vulkan_engine/src")
add_subdirectory("sandbox/src")
