#include <iostream>
#include "utils.hpp"
#include "parsers.hpp"

using inspector::utils::exec;
using inspector::utils::string_format;
using inspector::parser::parsed_inspectors_output;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "error: wrong argument number: executable file is required" << std::endl;
        return 1;
    }
    std::string executable = std::string(argv[1]);
    // TODO: add possibility to pass "-csv_result" flag as an argument
    bool csv_result = true;
    std::string stack_inspector_output = exec(
            string_format(
                    "%s %s",
                    STACK_INSPECTOR_EXEC,
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
        if (csv_result) {
            std::cout << result.as_csv() << std::endl;
        } else {
            std::cout << result << std::endl;
        }
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
