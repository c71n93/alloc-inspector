#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

// TODO: get rid of such copy-paste
template<typename T>
concept formattable =
std::is_fundamental<T>::value ||
(std::is_pointer<T>::value && std::is_fundamental<std::remove_pointer_t<T>>::value);
std::string string_format(const std::string &format, formattable auto ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) {
        throw std::runtime_error("error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "error: wrong argument number: executable file is required" << std::endl;
        return 1;
    }
    std::string executable = std::string(argv[1]);
    // TODO: add possibility to pass "-only_from_app" flag as an argument
    std::cout << string_format(
            "%s -c %s -only_from_app -- %s",
            DRRUN_EXECUTABLE,
            STACK_INSPECTOR,
            executable.c_str()
    ) << std::endl;
    return std::system(
        string_format(
            "%s -c %s -only_from_app -- %s",
            DRRUN_EXECUTABLE,
            STACK_INSPECTOR,
            executable.c_str()
        ).c_str()
    );
}
