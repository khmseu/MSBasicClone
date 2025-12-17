cmake_minimum_required(VERSION 3.20)
project(MSBasicClone VERSION 1.0.0 LANGUAGES CXX)

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

# Create executable
add_executable(msbasic ${SOURCES} ${HEADERS})

# Link math library on Unix-like systems
if(UNIX)
    target_link_libraries(msbasic m)
endif()

# Installation
install(TARGETS msbasic DESTINATION bin)
