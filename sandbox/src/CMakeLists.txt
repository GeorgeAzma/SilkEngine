project(Sandbox VERSION 1.0.0 LANGUAGES CXX)

file(GLOB_RECURSE SANDBOX_SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
add_executable(Sandbox ${SANDBOX_SOURCES})
target_include_directories(Sandbox PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(Sandbox PRIVATE SilkEngine)

#Copy data folder where exe file is
add_custom_command(TARGET Sandbox POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory 
${CMAKE_SOURCE_DIR}/data 
$<TARGET_FILE_DIR:Sandbox>/data)
target_compile_definitions(SilkEngine PRIVATE "SK_CLIENT")
