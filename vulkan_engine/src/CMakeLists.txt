file(GLOB_RECURSE VULKAN_ENGINE_SOURCES "*.cpp" "*.h" "*.hpp")

#Create VulkanEngine.lib
add_library(VulkanEngine STATIC ${VULKAN_ENGINE_SOURCES})

#Include dirs
target_include_directories(VulkanEngine PUBLIC 
"${PROJECT_SOURCE_DIR}/vendor" 
"${PROJECT_SOURCE_DIR}/vulkan_engine/src"
"E:/Resources/CodingResources/Vulkan/Include"
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

#Link
target_link_directories(VulkanEngine PUBLIC ${EXTRA_LIB_DIRS})
target_link_libraries(VulkanEngine ${EXTRA_LIBS})

#Precompile headers
target_precompile_headers(VulkanEngine PUBLIC 
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
core/base.h
core/log.h)