#include <iostream>
#include "utils.hpp"
#include "parsers.hpp"
#include <csignal>

enum ret_code {
    STACK_INSPECTOR_TIMEOUT = 1,
    VALGRIND_TIMEOUT = 2,
    STACK_INSPECTOR_PARSING_ERROR = 3,
    VALGRIND_PARSING_ERROR = 4,
};

using inspector::utils::exec;
using inspector::utils::string_format;
using inspector::parser::parsed_inspectors_output;
using inspector::utils::exec_result;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "error: wrong argument number: executable file is required" << std::endl;
        return 0;
    }
    std::string executable = std::string(argv[1]);
    // TODO: add possibility to pass "-csv_result" flag as an argument
    bool csv_result = true;
    // TODO: add possibility to pass timelimit in seconds
    int timelimit = 1200;
    exec_result stack_inspector_result = exec(
        string_format(
            "timeout --signal=SIGKILL %d %s %s",
            timelimit,
            STACK_INSPECTOR_EXEC,
            executable.c_str()
        )
    );
    if (stack_inspector_result.exit_code == SIGKILL) {
        std::cerr << "stack_inspector is killed due to exceeding the time limit of "
                  << timelimit << "s" << std::endl;
        return ret_code::STACK_INSPECTOR_TIMEOUT;
    }
    // TODO: add possibility to pass --multiprocess to enable --trace-children=yes if needed
    exec_result valgrind_result = exec(
        string_format(
            "timeout --signal=SIGKILL %d valgrind --trace-children=yes %s 2>&1",
            timelimit,
            executable.c_str()
        )
    );
    // TODO: why valgrind not returning SIGKILL when killed?
    if (stack_inspector_result.exit_code == SIGKILL) {
        std::cerr << "valgrind is killed due to exceeding the time limit of "
                  << timelimit << "s" << std::endl;
        return ret_code::VALGRIND_TIMEOUT;
    }
    try {
        parsed_inspectors_output::inspectors_result result = parsed_inspectors_output(
                stack_inspector_result.output,
                valgrind_result.output
        ).take();
        if (csv_result) {
            std::cout << result.as_csv() << std::endl;
        } else {
            std::cout << result << std::endl;
        }
    } catch (const inspector::parser::stack_inspector_out_format_error& e) {
        std::cerr << "error: unexpected output from stack_inspector:\n"
                  << stack_inspector_result.output << "\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::STACK_INSPECTOR_PARSING_ERROR;
    } catch (const inspector::parser::valgrind_out_format_error& e) {
        std::cerr << "error: unexpected output from valgrind:\n"
                  << valgrind_result.output << "\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::VALGRIND_PARSING_ERROR;
    }
    return 0;
}
