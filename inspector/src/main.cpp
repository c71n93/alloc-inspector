#include <iostream>
#include "utils.hpp"

using inspector::utils::exec;
using inspector::utils::string_format;

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
    std::cout <<
        exec(string_format(
                "%s -c %s %s -- %s",
                DRRUN_EXECUTABLE,
                STACK_INSPECTOR,
                only_from_app_opt.c_str(),
                executable.c_str())
        )
        << std::endl;
    std::cout << exec(string_format("valgrind %s", executable.c_str())) << std::endl;
    return 0;
}
