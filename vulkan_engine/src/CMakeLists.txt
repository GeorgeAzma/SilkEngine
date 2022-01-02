file(GLOB_RECURSE VULKAN_ENGINE_SOURCES "*.cpp")

#Create VulkanEngine.lib
add_library(VulkanEngine STATIC ${VULKAN_ENGINE_SOURCES})

#Defines
target_compile_definitions(VulkanEngine PUBLIC 
"$<$<CONFIG:DEBUG>:VE_DEBUG>"
"$<$<CONFIG:RELEASE>:VE_DIST>"
)

#Link
target_link_libraries(VulkanEngine ${EXTRA_LIBS})

#Precompile headers
target_precompile_headers(VulkanEngine PUBLIC "core/pch.h")