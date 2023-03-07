project(Tests LANGUAGES CXX)

set(TEST_PROJECT_NAME tests)

add_subdirectory(googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES *.cpp)
add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})
add_dependencies(${TEST_PROJECT_NAME} ${ENGINE_NAME} ${MAIN_PROJECT_NAME})
target_include_directories(${ENGINE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TEST_PROJECT_NAME} PUBLIC gtest gtest_main ${ENGINE_NAME} ${MAIN_PROJECT_NAME})

set(RESOURCE_PATH ${CMAKE_SOURCE_DIR}/res)
file(COPY ${RESOURCE_PATH} DESTINATION ${PROJECT_BINARY_DIR})

include(GoogleTest)
gtest_discover_tests(${TEST_PROJECT_NAME})