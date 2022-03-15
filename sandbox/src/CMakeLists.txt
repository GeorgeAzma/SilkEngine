project(Sandbox VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE SANDBOX_SOURCES "*.cpp")
add_executable(Sandbox ${SANDBOX_SOURCES})
message(${SANDBOX_SOURCES})
target_link_libraries(Sandbox PRIVATE SilkEngine)

#Copy data folder where exe file is
add_custom_command(TARGET Sandbox POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory 
${CMAKE_SOURCE_DIR}/data 
$<TARGET_FILE_DIR:Sandbox>/data)
