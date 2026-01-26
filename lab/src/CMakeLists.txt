cmake_minimum_required(VERSION 3.29)

project(curl_lab)

set(CMAKE_INCLUDE_CURRENT_DIR OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if CMAKE_CONFIGURATION_TYPES is defined
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    message(STATUS "[${PROJECT_NAME}] CMAKE_CONFIGURATION_TYPES is defined: ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "[${PROJECT_NAME}] CMAKE_CONFIGURATION_TYPES is not defined.")
endif()

# Separate multiple Projects and put them into folders which are on top-level.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# How do I make CMake output into a 'bin' dir?
#   The correct variable to set is CMAKE_RUNTIME_OUTPUT_DIRECTORY.
#   We use the following in our root CMakeLists.txt:
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(USE_STATIC_LIBS)
    # use static link
    if(WIN32)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(WIN32)
    # Target Windows 7
    add_definitions(-D_WIN32_WINNT=0x601)

    # unicode
    add_definitions(-DUNICODE -D_UNICODE)

    # use mfc dll
    # add_definitions(-D_AFXDLL)
  
    # add compiler options
    add_compile_options(/Zi /utf-8)
    # add_compile_options(/wd4251 /wd4996)
  
    # add linker options
    add_link_options(/DEBUG)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party lab_third_party)

add_subdirectory(demo)
