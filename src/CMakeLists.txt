set(CREATE_INSTALLER OFF)
set(PROJECT test_project)

project(${PROJECT} VERSION 1.0.0 LANGUAGES CXX)

add_executable(${PROJECT} "")

set(VS_STARTUP_PROJECT ${PROJECT})

file(GLOB_RECURSE PROJECT_SOURCES "*.cpp")
target_sources(${PROJECT} PRIVATE ${PROJECT_SOURCES})

target_link_libraries(${PROJECT} ${ENGINE_NAME})
target_compile_definitions(${ENGINE_NAME} PRIVATE "SK_CLIENT")

set(RESOURCE_PATH ${CMAKE_SOURCE_DIR}/res)
file(COPY ${RESOURCE_PATH} DESTINATION ${PROJECT_BINARY_DIR})

if(CREATE_INSTALLER)
    set(ICON_PATH ${RESOURCE_PATH}/images/icons\\\\icon.ico)
    set(LICENSE_PATH ${RESOURCE_PATH}/text/License.txt)
    install(TARGETS ${PROJECT} DESTINATION .)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/res DESTINATION .)
    if(EXISTS ${ICON_PATH})
        set(CPACK_NSIS_MUI_ICON ${ICON_PATH})
        set(CPACK_NSIS_MUI_UNIICON ${ICON_PATH})
        set(CPACK_NSIS_INSTALLED_ICON_NAME ${ICON_PATH})
    endif()
    if(EXISTS ${LICENSE_PATH})
        set(CPACK_RESOURCE_FILE_LICENSE ${LICENSE_PATH})
    endif()
    #set(CPACK_PACKAGE_ICON ${ICON_PATH})
    include(CPack)
endif()