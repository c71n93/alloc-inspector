#include <iostream>
#include "utils.hpp"
#include "parsers.hpp"

using inspector::utils::exec;
using inspector::utils::string_format;
using inspector::parser::parsed_inspectors_output;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "error: wrong argument number: executable file is required" << std::endl;
        return 1;
    }
    std::string executable = std::string(argv[1]);
    // TODO: add possibility to pass "-only_from_app" flag as an argument
    std::string only_from_app_opt = "";
    if (true) {
        only_from_app_opt = "-only_from_app";
    }
    std::string stack_inspector_output = exec(
            string_format(
                    "%s -c %s %s -- %s",
                    DRRUN_EXECUTABLE,
                    STACK_INSPECTOR,
                    only_from_app_opt.c_str(),
                    executable.c_str()
            )
    );
    std::string valgrind_output = exec(
            string_format("valgrind %s 2>&1", executable.c_str())
    );
    try {
        parsed_inspectors_output::inspectors_result result = parsed_inspectors_output(
            stack_inspector_output,
            valgrind_output
        ).parse_and_take();
        std::cout << result << std::endl;
    } catch (const inspector::parser::stack_inspector_out_format_error& e) {
        std::cerr << "error: unexpected output from stack_inspector:\n"
                  << stack_inspector_output << "\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
    } catch (const inspector::parser::valgrind_out_format_error& e) {
        std::cerr << "error: unexpected output from valgrind:\n"
                  << valgrind_output << "\n"
                  << "the error was caused by:\n" << e.what() << std::endl;
    }
    return 0;
}
