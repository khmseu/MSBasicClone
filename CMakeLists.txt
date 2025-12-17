cmake_minimum_required(VERSION 3.20)

set(DEFAULT_VERSION "1.0.0")
set(MSBASIC_VERSION_STRING "${DEFAULT_VERSION}")

if(DEFINED VERSION_OVERRIDE)
    set(MSBASIC_VERSION_STRING "${VERSION_OVERRIDE}")
else()
    find_package(Git QUIET)
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            RESULT_VARIABLE GIT_DESCRIBE_RESULT
            OUTPUT_VARIABLE GIT_DESCRIBE_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(GIT_DESCRIBE_RESULT EQUAL 0 AND NOT "${GIT_DESCRIBE_OUTPUT}" STREQUAL "")
            set(MSBASIC_VERSION_STRING "${GIT_DESCRIBE_OUTPUT}")
        endif()
    endif()
endif()

project(MSBasicClone LANGUAGES CXX)
message(STATUS "MSBasicClone version: ${MSBASIC_VERSION_STRING}")

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PLATFORM_MACOS)
elseif(UNIX)
    add_compile_definitions(PLATFORM_LINUX)
endif()

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall
        -Wextra
        -pedantic
        -O3  # Performance optimization
    )
endif()

if(MSVC)
    add_compile_options(
        /W4
        /O2
    )
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/types.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/interpreter.cpp
    src/variables.cpp
    src/functions.cpp
    src/statements.cpp
    src/filesystem.cpp
    src/interactive.cpp
    src/float40.cpp
)

# Header files
set(HEADERS
    src/tokenizer.h
    src/parser.h
    src/interpreter.h
    src/variables.h
    src/functions.h
    src/statements.h
    src/filesystem.h
    src/interactive.h
    src/float40.h
    src/types.h
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h
    @ONLY
)

set(CPACK_PACKAGE_VERSION "${MSBASIC_VERSION_STRING}")
include(CPack)

# Create executable
add_executable(msbasic ${SOURCES} ${HEADERS})
target_include_directories(msbasic PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

enable_testing()

file(GLOB BAS_TEST_FILES
    "${CMAKE_SOURCE_DIR}/tests/*.bas"
    "${CMAKE_SOURCE_DIR}/examples/*.bas"
)

foreach(BAS_FILE ${BAS_TEST_FILES})
    get_filename_component(BAS_NAME "${BAS_FILE}" NAME_WE)
    add_test(
        NAME bas_${BAS_NAME}
        COMMAND $<TARGET_FILE:msbasic> ${BAS_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endforeach()

# Link math library on Unix-like systems
if(UNIX)
    target_link_libraries(msbasic m)
endif()

# Installation
install(TARGETS msbasic DESTINATION bin)
