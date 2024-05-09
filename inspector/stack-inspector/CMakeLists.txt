add_library(stack_inspector SHARED src/stack-inspector.cpp)
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
if (NOT DEFINED DynamoRIO_DIR OR "${DynamoRIO_DIR}" STREQUAL "DynamoRIO_DIR-NOTFOUND")
    set(DynamoRIO_DIR ${DYNAMORIO_HOME}/cmake)
endif (NOT DEFINED DynamoRIO_DIR OR "${DynamoRIO_DIR}" STREQUAL "DynamoRIO_DIR-NOTFOUND")
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
    message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(stack_inspector drmgr)
use_DynamoRIO_extension(stack_inspector droption)
configure_DynamoRIO_client(stack_inspector)

set(DRRUN_EXECUTABLE ${DYNAMORIO_HOME}/bin64/drrun)
set(STACK_INSPECTOR $<TARGET_FILE:stack_inspector>)

add_subdirectory(test)
