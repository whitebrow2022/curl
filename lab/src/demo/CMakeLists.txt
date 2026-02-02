project(curl_demo)

set(APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE APP_SOURCES
    ${APP_SRC_DIR}/*.h
    ${APP_SRC_DIR}/*.cc
    )

source_group(TREE ${APP_SRC_DIR} PREFIX "src" FILES ${APP_SOURCES})

add_executable(${PROJECT_NAME} ${APP_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE libcurl_static)
target_link_libraries(${PROJECT_NAME} PRIVATE winhttp)

if(CURL_USE_SCHANNEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SCHANNEL)
else(CURL_USE_BORINGSSL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_BORINGSSL)
endif()

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
       WIN32_EXECUTABLE FALSE
       )
    if(REQUIRE_ADMINISTRATOR)
        target_link_options(${PROJECT_NAME}
            PRIVATE
            "/MANIFESTUAC:level='requireAdministrator'"
            "/MANIFESTUAC:uiAccess='false'"
        )
    endif()

    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()
