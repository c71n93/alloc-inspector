cmake_minimum_required(VERSION 3.7)
project(inspector)

set(CMAKE_CXX_STANDARD 20)

if(NOT DEFINED DYNAMORIO_HOME)
    if(DEFINED ENV{DYNAMORIO_HOME})
        set(DYNAMORIO_HOME $ENV{DYNAMORIO_HOME})
    else()
        message(
                FATAL_ERROR
                "The DYNAMORIO_HOME variable must be specified via -D or via the environment variable of the same name"
        )
    endif()
endif(NOT DEFINED DYNAMORIO_HOME)
set(DRRUN_EXECUTABLE ${DYNAMORIO_HOME}/bin64/drrun)

enable_testing()
add_subdirectory(stack-inspector)

# TODO: make alloc-inspector depend on stack-inspector
add_executable(alloc-inspector src/main.cpp)
#add_dependencies(alloc-inspector stack-inspector)
target_include_directories(alloc-inspector PUBLIC include)
target_compile_definitions(alloc-inspector
    PUBLIC DRRUN_EXECUTABLE="${DRRUN_EXECUTABLE}"
    PUBLIC STACK_INSPECTOR="$<TARGET_FILE:stack_inspector>"
)
