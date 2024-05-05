#include <iostream>
#include "utils.hpp"
#include "parsers.hpp"
#include "subprocess.hpp"
#include <csignal>

enum ret_code {
    STACK_INSPECTOR_TIMEOUT = 1,
    VALGRIND_TIMEOUT = 2,
    STACK_INSPECTOR_PARSING_ERROR = 3,
    VALGRIND_PARSING_ERROR = 4,
};

namespace sp = subprocess;
using inspector::utils::string_format;
using inspector::parser::parsed_inspectors_output;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "error: wrong argument number: executable file is required" << std::endl;
        return 0;
    }
    std::string executable = std::string(argv[1]);
    // TODO: add possibility to pass "-csv_result" flag as an argument
    bool csv_result = true;
    // TODO: add possibility to pass timelimit in seconds
    int timelimit = 100;
    auto stack_inspector_process = sp::Popen(
        string_format(
            "timeout --signal=SIGKILL %d %s %s",
            timelimit,
            STACK_INSPECTOR_EXEC,
            executable.c_str()
       ),
       sp::output{sp::PIPE}
    );
    std::string stack_inspector_output{stack_inspector_process.communicate().first.buf.data()};
    int stack_inspector_retcode = stack_inspector_process.retcode();
    if (stack_inspector_retcode == SIGKILL) {
        std::cerr << "stack_inspector is killed due to exceeding the time limit of "
                  << timelimit << "s" << std::endl;
        return ret_code::STACK_INSPECTOR_TIMEOUT;
    }
    // TODO: add possibility to pass --multiprocess to enable --trace-children=yes if needed
    auto valgrind_process = sp::Popen(
        string_format(
            "timeout --signal=SIGKILL %d valgrind --trace-children=yes %s",
            timelimit,
            executable.c_str()
        ),
        sp::error{sp::PIPE}
    );
    std::string valgrind_output{valgrind_process.communicate().second.buf.data()};
    auto valgrind_retcode = valgrind_process.retcode();
    if (valgrind_retcode == SIGKILL) {
        std::cerr << "valgrind is killed due to exceeding the time limit of "
                  << timelimit << "s" << std::endl;
        return ret_code::VALGRIND_TIMEOUT;
    }
    try {
        parsed_inspectors_output::inspectors_result result = parsed_inspectors_output(
                stack_inspector_output,
                valgrind_output
        ).take();
        if (csv_result) {
            std::cout << result.as_csv() << std::endl;
        } else {
            std::cout << result << std::endl;
        }
    } catch (const inspector::parser::stack_inspector_out_format_error& e) {
        std::cerr << "error: unexpected output from stack_inspector:\n"
                  << stack_inspector_output << "\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::STACK_INSPECTOR_PARSING_ERROR;
    } catch (const inspector::parser::valgrind_out_format_error& e) {
        std::cerr << "error: unexpected output from valgrind:\n"
                  << valgrind_output << "\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::VALGRIND_PARSING_ERROR;
    }
    return 0;
}
