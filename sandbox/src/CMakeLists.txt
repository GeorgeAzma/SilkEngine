file(GLOB_RECURSE SANDBOX_SOURCES "*.cpp")

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox PRIVATE VulkanEngine)