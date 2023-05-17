set(ENGINE_NAME SilkEngine CACHE STRING "" FORCE)

project(${ENGINE_NAME} VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE SILK_ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(${ENGINE_NAME} STATIC ${SILK_ENGINE_SOURCES})
target_include_directories(${ENGINE_NAME} PUBLIC ${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
if(DEPENDENCIES_TO_ADD)
	add_dependencies(${ENGINE_NAME} ${DEPENDENCIES_TO_ADD})
endif()

#Defines
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${ENGINE_NAME} PUBLIC "SK_DIST")
else()
	target_compile_definitions(${ENGINE_NAME} PUBLIC "SK_DEBUG")
endif()

target_compile_definitions(${ENGINE_NAME} PRIVATE "SK_CORE")
target_compile_definitions(${ENGINE_NAME} INTERFACE "SK_CLIENT")

#Precompile headers
target_precompile_headers(${ENGINE_NAME} PUBLIC [["core/base.h"]])
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

#Link
target_link_libraries(${ENGINE_NAME} PUBLIC ${EXTRA_LIBS})
