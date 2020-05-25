cmake_minimum_required(VERSION 3.0)

project(simple_http_server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-std=c++11 -Wall -Wextra -Wsign-conversion -O3)

# This option causes the compiler to abort compilation on the first error occurred rather than trying to keep going and printing further error messages
add_compile_options(-Wfatal-errors)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)
add_library(simplehttpserver SHARED ${SRC_FILES})

add_executable(example1 examples/example1.cc)
target_link_libraries(example1 simplehttpserver pthread)
