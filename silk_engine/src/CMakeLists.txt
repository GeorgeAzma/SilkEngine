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
<functional>
<algorithm>
<map>
<filesystem>
<fstream>
<mutex>
<optional>

#Less useful
<atomic>
<chrono>
<initializer_list>
<exception>
<type_traits>
<typeinfo>
<typeindex>
<unordered_set>
<set>
<queue>
<sstream>
<future>
<concepts>

#Non std
<glm/glm.hpp>
<glm/gtx/norm.hpp> # length2(), distance2()
<glm/gtc/constants.hpp> # pi, etc
<glm/gtc/matrix_transform.hpp> # ortho() perspective() etc.
<vulkan/vulkan.h>
<vk_mem_alloc.h>
<GLFW/glfw3.h>

#project sources
[["core/base.h"]] #Very rarely changed
[["core/log.h"]] #Not changed
[["utils/RNG.h"]] #Not changed
[["utils/math.h"]] #Rarely changed, but useful
)
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

#Link
target_link_libraries(SilkEngine ${EXTRA_LIBS})
