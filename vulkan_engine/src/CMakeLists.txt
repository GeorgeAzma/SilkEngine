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
if (DEFINED VULKAN_SDK_PATH)
    list(APPEND INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    list(APPEND EXTRA_LIB_DIRS "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND True)
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if(NOT Vulkan_FOUND)
    message("Could not find vulkan")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()
target_link_libraries(VulkanEngine Vulkan::Vulkan)

#Precompile headers
target_precompile_headers(VulkanEngine PUBLIC "core/pch.h")