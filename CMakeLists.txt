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

# Include font fetching module
include(cmake/FetchFont.cmake)

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
    src/graphics.cpp
    src/graphics_renderer.cpp
    src/tape_manager.cpp
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
    src/graphics.h
    src/graphics_config.h
    src/graphics_renderer.h
    src/tape_manager.h
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h
    @ONLY
)

set(CPACK_PACKAGE_VERSION "${MSBASIC_VERSION_STRING}")
include(CPack)

# Option to build raylib from source if not found
option(AUTO_BUILD_RAYLIB "Automatically build raylib from source if not found" ON)

# Optional: Find Raylib for graphics support
find_package(raylib QUIET)
if(raylib_FOUND)
    message(STATUS "Raylib found - graphics rendering enabled")
    add_compile_definitions(HAVE_RAYLIB)
    set(RAYLIB_AVAILABLE TRUE)
else()
    message(STATUS "Raylib not found via find_package")
    
    if(AUTO_BUILD_RAYLIB)
        message(STATUS "Attempting to build raylib from source...")
        
        # Check for X11 on Unix-like systems (required for raylib)
        if(UNIX AND NOT APPLE)
            find_package(X11)
            if(NOT X11_FOUND)
                message(WARNING "X11 development libraries not found. Raylib requires X11 on Linux.")
                message(WARNING "Install with: sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev")
                message(STATUS "Skipping raylib build - graphics will use off-screen buffer only")
                set(RAYLIB_AVAILABLE FALSE)
            endif()
        endif()
        
        if(NOT DEFINED RAYLIB_AVAILABLE OR RAYLIB_AVAILABLE)
            include(FetchContent)
            
            FetchContent_Declare(
                raylib
                GIT_REPOSITORY https://github.com/raysan5/raylib.git
                GIT_TAG 5.0
                GIT_SHALLOW TRUE
            )
            
            # Configure raylib build options
            set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
            
            # Attempt to build raylib
            FetchContent_MakeAvailable(raylib)
            
            if(TARGET raylib)
                message(STATUS "Successfully built raylib from source - graphics rendering enabled")
                add_compile_definitions(HAVE_RAYLIB)
                set(RAYLIB_AVAILABLE TRUE)
            else()
                message(STATUS "Failed to build raylib - graphics will use off-screen buffer only")
                set(RAYLIB_AVAILABLE FALSE)
            endif()
        endif()
    else()
        message(STATUS "AUTO_BUILD_RAYLIB is OFF - graphics will use off-screen buffer only")
        set(RAYLIB_AVAILABLE FALSE)
    endif()
endif()

# Fetch Apple II Font and charset map
fetch_apple2_font()

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

# Link Raylib if available
if(RAYLIB_AVAILABLE)
    target_link_libraries(msbasic raylib)
endif()

# Documentation target (requires Doxygen)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    message(STATUS "Doxygen found - documentation target available")
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found - documentation target unavailable")
    message(STATUS "Install Doxygen to build documentation: apt-get install doxygen (Ubuntu) or brew install doxygen (macOS)")
endif()

# Installation
install(TARGETS msbasic DESTINATION bin)
