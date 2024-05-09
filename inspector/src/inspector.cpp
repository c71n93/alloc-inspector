#include <iostream>
#include "util.hpp"
#include "subprocess.hpp"
#include "stack-inspector-launcher.hpp"
#include "valgrind-launcher.hpp"

enum ret_code {
    STACK_INSPECTOR_TIMEOUT = 1,
    VALGRIND_TIMEOUT = 2,
    STACK_INSPECTOR_PARSING_ERROR = 3,
    VALGRIND_PARSING_ERROR = 4,
};

namespace sp = subprocess;
using namespace inspector;
using util::stringFormat;
using stack_inspector::StackInspectorLauncher;
using valgrind::ValgrindLauncher;

// TODO: add tests
int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "error: wrong argument number: executable file is required" << std::endl;
        return 0;
    }
    std::string executable = std::string(argv[1]);
    // TODO: add possibility to pass "-csv_result" flag as an argument
    bool csv_result = false;
    // TODO: add possibility to pass timelimit in seconds
    size_t timelimit = 100;
    try {
        results::InspectorResults result{
            StackInspectorLauncher{
                timelimit,
                DRRUN_EXECUTABLE,
                STACK_INSPECTOR,
                executable
            }.launchWithResults(),
            ValgrindLauncher{
                timelimit,
                executable
            }.launchWithResults()
        };
        if (csv_result) {
            std::cout << result.as_csv() << std::endl;
        } else {
            std::cout << result << std::endl;
        }
    } catch (const stack_inspector::stack_inspector_timeout_error& e) {
        std::cerr << "error: stack_inspector timeout:\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::STACK_INSPECTOR_TIMEOUT;
    } catch (const valgrind::valgrind_timeout_error& e) {
        std::cerr << "error: valgrind timeout:\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::VALGRIND_TIMEOUT;
    } catch (const stack_inspector::stack_inspector_out_format_error& e) {
        std::cerr << "error: unexpected output from stack_inspector:\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::STACK_INSPECTOR_PARSING_ERROR;
    } catch (const valgrind::valgrind_out_format_error& e) {
        std::cerr << "error: unexpected output from valgrind:\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
        return ret_code::VALGRIND_PARSING_ERROR;
    }
    return 0;
}
