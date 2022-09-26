set(ENGINE_NAME SilkEngine CACHE INTERNAL "engine_name")

project(${ENGINE_NAME} VERSION 1.0.0 LANGUAGES CXX)

list(APPEND INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE SILK_ENGINE_SOURCES "src/*.cpp")
add_library(${ENGINE_NAME} STATIC ${SILK_ENGINE_SOURCES})
target_include_directories(${ENGINE_NAME} PUBLIC ${INCLUDE_DIRS})
add_dependencies(${ENGINE_NAME} ${DEPENDENCIES_TO_ADD})

#Defines
target_compile_definitions(${ENGINE_NAME} PUBLIC 
"$<$<CONFIG:DEBUG>:SK_DEBUG>"
"$<$<CONFIG:RELEASE>:SK_DIST>"
)
target_compile_definitions(${ENGINE_NAME} PRIVATE "SK_CORE")

#Precompile headers
target_precompile_headers(SilkEngine PUBLIC [["core/base.h"]])
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

#Link
target_link_libraries(${ENGINE_NAME} ${EXTRA_LIBS})
