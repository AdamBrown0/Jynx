cmake_minimum_required(VERSION 3.20)

project(jynxc LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W3)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(
    JYNX_LOG_LEVEL=Log::Level::DEBUG
)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(CORE_SOURCES
    src/lexer.cc
    src/parser.cc
    src/sema.cc
    src/log.cc
    src/token.cc
    src/scope.cc
    src/context.cc
    # src/gen.cc
    src/primitive_type.cc
    src/visitor/typechecker.cc
    src/visitor/symbolcollector.cc
    src/visitor/nameresolver.cc
)

set(MAIN_SOURCES
    src/main.cc
)

add_library(jynxcore STATIC ${CORE_SOURCES})
target_include_directories(jynxcore PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(jynxc ${MAIN_SOURCES})
target_link_libraries(jynxc PRIVATE jynxcore)

set_target_properties(jynxc PROPERTIES OUTPUT_NAME "jynxc")


# tests
include(CTest)

file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.jx")

set(EXPECT_FAIL_TESTS
  "vardecl_wrong_types"
  "example"
)

foreach(test_file IN LISTS TEST_FILES)
  get_filename_component(test_name "${test_file}" NAME_WE)

  add_test(NAME "full_${test_name}" COMMAND jynxc "${test_file}")

  if(test_name IN_LIST EXPECT_FAIL_TESTS)
    set_tests_properties("full_${test_name}" PROPERTIES WILL_FAIL TRUE)
  endif()
endforeach()
