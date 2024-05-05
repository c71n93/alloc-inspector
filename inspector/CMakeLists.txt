cmake_minimum_required(VERSION 3.7)
project(inspector)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
add_subdirectory(stack-inspector)
add_subdirectory(third-party)

add_library(alloc_inspector_include INTERFACE)
target_include_directories(alloc_inspector_include INTERFACE include)

add_executable(alloc_inspector src/main.cpp)
target_link_libraries(alloc_inspector PUBLIC alloc_inspector_include subprocess)
target_compile_definitions(alloc_inspector
    PUBLIC STACK_INSPECTOR_EXEC="$<TARGET_FILE:stack_inspector_exec>"
)
