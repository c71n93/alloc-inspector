#include "iostream"
#include "util.hpp"
#include "stack-inspector-launcher.hpp"

using namespace inspector;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "error: wrong argument number: executable file is required" << std::endl;
        return 1;
    }
    std::string executable = std::string(argv[1]);
    size_t timelimit = 100;
    results::StackInspectorResults result = stack_inspector::StackInspectorLauncher{
            timelimit,
            DRRUN_EXECUTABLE,
            STACK_INSPECTOR,
            executable
        }.launchWithResults();
    std::cout << result << std::endl;
    return 0;
}
