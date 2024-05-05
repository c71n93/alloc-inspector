#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <array>

namespace inspector::utils {

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

}
