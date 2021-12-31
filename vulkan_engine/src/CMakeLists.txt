file(GLOB_RECURSE VULKAN_ENGINE_SOURCES "*.cpp")

#Create VulkanEngine.lib
add_library(VulkanEngine STATIC ${VULKAN_ENGINE_SOURCES})

#Defines
target_compile_definitions(VulkanEngine PUBLIC 
"$<$<CONFIG:DEBUG>:DEBUG>"
"$<$<CONFIG:DEBUG>:_DEBUG>"
"$<$<CONFIG:RELEASE>:DIST>"
)

#Link
target_link_libraries(VulkanEngine Vulkan::Vulkan Threads::Threads)

#Precompile headers
target_precompile_headers(VulkanEngine PUBLIC "core/pch.h")