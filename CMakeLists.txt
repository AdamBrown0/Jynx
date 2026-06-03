cmake_minimum_required(VERSION 3.20)

project(jynxc LANGUAGES CXX C)

# ========================== Options ==========================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Warning level 3
if (MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Address sanitizer
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ========================== Directories ==========================
include_directories(${CMAKE_SOURCE_DIR}/include)

# ========================== Source Files ==========================
set(CORE_SOURCES
    src/lexer.cc
    src/parser.cc
    # src/sema.cc
    src/log.cc
    src/token.cc
    src/context.cc
    # src/gen.cc
    src/primitive_type.cc
    src/visitor/typechecker.cc
    src/visitor/symbolcollector.cc
    # src/visitor/nameresolver.cc
)

set(MAIN_SOURCES
    src/main.cc
)

# ========================== Targets ==========================

# Core static library
add_library(jynxcore STATIC ${CORE_SOURCES})
target_include_directories(jynxcore PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Main executable
add_executable(jynxc ${MAIN_SOURCES})
target_link_libraries(jynxc PRIVATE jynxcore)

# Optional: Set output name
set_target_properties(jynxc PROPERTIES OUTPUT_NAME "jynxc")
