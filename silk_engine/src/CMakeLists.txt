project(SilkEngine VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE SILK_ENGINE_SOURCES "*.cpp")
add_library(SilkEngine STATIC ${SILK_ENGINE_SOURCES})
target_include_directories(SilkEngine PUBLIC ${INCLUDE_DIRS} ${PROJECT_SOURCE_DIR})
add_dependencies(SilkEngine ${DEPENDENCIES_TO_ADD})

#Defines
target_compile_definitions(SilkEngine PUBLIC 
"$<$<CONFIG:DEBUG>:SK_DEBUG>"
"$<$<CONFIG:RELEASE>:SK_DIST>"
)
target_compile_definitions(SilkEngine PRIVATE "SK_CORE")
if(MSVC AND NOT CMAKE_BUILD_TYPE EQUAL "RELEASE")
  target_link_options(SilkEngine PUBLIC "/INCREMENTAL")
endif()

#Precompile headers
target_precompile_headers(SilkEngine PUBLIC
#Very useful
<memory>
<cmath>
<vector>
<array>
<string>
<unordered_map>
<thread>

#Less useful
<unordered_set>
<set>
<map>
<filesystem>
<fstream>
<functional>
<algorithm>
<queue>
<mutex>

#Non std
<glm/glm.hpp>

#project sources
core/base.h
core/log.h
utils/RNG.h
utils/math.h
)

#Link
target_link_libraries(SilkEngine ${EXTRA_LIBS})
