cmake_minimum_required(VERSION 3.7)
project(inspector)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
add_subdirectory(stack-inspector)

add_executable(alloc_inspector src/main.cpp)
target_include_directories(alloc_inspector PUBLIC include)
target_compile_definitions(alloc_inspector
    PUBLIC STACK_INSPECTOR_EXEC="$<TARGET_FILE:stack_inspector_exec>"
)
