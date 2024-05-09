cmake_minimum_required(VERSION 3.7)
project(inspector)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
add_subdirectory(stack-inspector)
get_directory_property(DRRUN_EXECUTABLE DIRECTORY stack-inspector DEFINITION DRRUN_EXECUTABLE)
add_subdirectory(third-party)

add_library(alloc_inspector_include INTERFACE)
target_include_directories(alloc_inspector_include INTERFACE include)
target_link_libraries(alloc_inspector_include INTERFACE subprocess)

add_executable(stack_inspector_exec src/stack-inspector-exec.cpp)
target_link_libraries(stack_inspector_exec PUBLIC alloc_inspector_include)
add_dependencies(stack_inspector_exec stack_inspector)
target_compile_definitions(stack_inspector_exec
        PUBLIC DRRUN_EXECUTABLE="${DRRUN_EXECUTABLE}"
        PUBLIC STACK_INSPECTOR="$<TARGET_FILE:stack_inspector>"
)

add_executable(alloc_inspector src/inspector.cpp)
target_link_libraries(alloc_inspector PUBLIC alloc_inspector_include)
add_dependencies(alloc_inspector stack_inspector)
target_compile_definitions(alloc_inspector
    PUBLIC DRRUN_EXECUTABLE="${DRRUN_EXECUTABLE}"
    PUBLIC STACK_INSPECTOR="$<TARGET_FILE:stack_inspector>"
)
