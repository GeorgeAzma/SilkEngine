file(GLOB_RECURSE VULKAN_ENGINE_SOURCES "*.cpp")

#Create VulkanEngine.lib
add_library(VulkanEngine STATIC ${VULKAN_ENGINE_SOURCES})

#Link
target_link_libraries(VulkanEngine Vulkan::Vulkan)

#Precompile headers
target_precompile_headers(VulkanEngine PUBLIC "pch.h")