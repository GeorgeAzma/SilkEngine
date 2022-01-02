file(GLOB_RECURSE VULKAN_ENGINE_SOURCES "*.cpp")

#Create VulkanEngine.lib
add_library(VulkanEngine STATIC ${VULKAN_ENGINE_SOURCES})

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
target_precompile_headers(VulkanEngine PUBLIC "core/pch.h")