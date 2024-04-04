find_package(Python COMPONENTS Interpreter REQUIRED)

function(add_integration_pytest SOURCE_NAME)
    get_filename_component(TEST_NAME ${SOURCE_NAME} NAME_WE)
    set(STACK_INSPECTOR_EXEC_LOC $<TARGET_FILE:stack_inspector>)
    set(TEST_EXEC_NAME ${TEST_NAME}-app)
    add_executable(${TEST_EXEC_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/resources/test-apps/${TEST_NAME}.cpp)
    set_target_properties(${TEST_EXEC_NAME} PROPERTIES COMPILE_FLAGS "-O0")
    set(TEST_EXEC_LOC $<TARGET_FILE:${TEST_EXEC_NAME}>)
    add_test(NAME ${TEST_NAME}
            COMMAND ${Python_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/python/${SOURCE_NAME}
            $<TARGET_FILE:stack_inspector_exec>
            ${TEST_EXEC_LOC})
endfunction(add_integration_pytest)

if (PYTHON_FOUND)
    file(GLOB FILES "${CMAKE_CURRENT_SOURCE_DIR}/python/*.py")
    foreach(FILE ${FILES})
        get_filename_component(FILE_NAME ${FILE} NAME)
        add_integration_pytest(${FILE_NAME})
    endforeach()
else ()
    message("Python was not found. Starting without testing.")
endif ()
