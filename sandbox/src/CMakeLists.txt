file(GLOB_RECURSE SANDBOX_SOURCES "*.cpp" "*.h" "*.hpp")

add_executable(Sandbox ${SANDBOX_SOURCES})

if(MSVC AND "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	set_target_properties(Sandbox PROPERTIES LINK_FLAGS 
	"/subsystem:windows 
	/ENTRY:mainCRTStartup")
endif()

target_link_libraries(Sandbox PRIVATE VulkanEngine)
