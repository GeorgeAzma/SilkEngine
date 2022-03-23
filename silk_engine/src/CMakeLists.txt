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
<glm/gtc/matrix_transform.hpp>
<glm/gtx/hash.hpp>
<glm/gtx/string_cast.hpp>

#project sources
core/base.h
core/log.h
utils/RNG.h
utils/thread_pool.h
utils/math.h
)

#Link
target_link_libraries(SilkEngine ${EXTRA_LIBS})
