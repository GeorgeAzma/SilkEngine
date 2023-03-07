set(ENGINE_NAME SilkEngine CACHE STRING "" FORCE)

project(${ENGINE_NAME} VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE SILK_ENGINE_SOURCES *.cpp)
add_library(${ENGINE_NAME} STATIC ${SILK_ENGINE_SOURCES})
target_include_directories(${ENGINE_NAME} PUBLIC ${INCLUDE_DIRS})
target_include_directories(${ENGINE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(${ENGINE_NAME} ${DEPENDENCIES_TO_ADD})

#Defines
if(${CONFIG_MODE} STREQUAL RELEASE)
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
