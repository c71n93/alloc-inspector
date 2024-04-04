#pragma once

#include <cstdio>
#include <stdexcept>
#include "utils.hpp"

namespace inspector::parser {

class format_error : public std::logic_error {
public:
    format_error(const char* msg) : std::logic_error(msg) {}
    format_error(const std::string& msg) : std::logic_error(msg) {}
    virtual ~format_error() {}
};

class stack_inspector_out_format_error final : public format_error {
public:
    stack_inspector_out_format_error(const char* msg) : format_error(msg) {}
    stack_inspector_out_format_error(const std::string& msg) : format_error(msg) {}
};

class valgrind_out_format_error final : public format_error {
public:
    valgrind_out_format_error(const char* msg) : format_error(msg) {}
    valgrind_out_format_error(const std::string& msg) : format_error(msg) {}
};

class parsed_inspectors_output {
public:
    struct inspectors_output {
        const std::string stack_inspector_output;
        const std::string valgrind_output;
    };

    struct inspectors_result {
        inspectors_result(size_t stack_allocs, size_t heap_allocs) :
            stack_allocs(stack_allocs), heap_allocs(heap_allocs) {}

        const size_t stack_allocs;
        const size_t heap_allocs;
        const double stack_allocs_fraction = stack_allocs + heap_allocs == 0? 0 :
                static_cast<double>(stack_allocs) / static_cast<double>(stack_allocs + heap_allocs);
        double heap_allocs_fraction = stack_allocs + heap_allocs == 0? 0 :
                static_cast<double>(heap_allocs) / static_cast<double>(stack_allocs + heap_allocs);

        std::string as_csv() {
            return utils::string_format("%lu,%lu,%lf,%lf", stack_allocs, heap_allocs, stack_allocs_fraction, heap_allocs_fraction);
        }
    };

    parsed_inspectors_output(
            const std::string& stack_inspector_output, const std::string& valgrind_output
    ) : output_{stack_inspector_output, valgrind_output} {}

    inspectors_result parse_and_take() {
        parse_stack_inspector();
        parse_valgrind();
        return inspectors_result{stack_allocs_, heap_allocs_};
    }

private:
    void parse_stack_inspector() {
        std::string info;
        try {
            info = obtain_line_from_string_that_starts_with(
                    output_.stack_inspector_output,
                    "Instrumentation results:"
            );
        } catch (const std::invalid_argument& e) {
            throw stack_inspector_out_format_error(e.what());
        }

        if (
            sscanf(
                info.c_str(),
                "Instrumentation results: %lu", &stack_allocs_
            ) == 0
        ) {
            throw stack_inspector_out_format_error(
                    "Wrong format of stack_inspector result output. "
                    "Unable to read instrumentation result."
            );
        }
    }

    void parse_valgrind() {
        std::string info;
        try {
            info = obtain_line_from_string_that_starts_with(
                    output_.valgrind_output,
                    "total heap usage:"
            );
        } catch (const std::invalid_argument& e) {
            throw valgrind_out_format_error(e.what());
        }
        if (
            sscanf(
                info.c_str(),
                "total heap usage: %lu", &heap_allocs_
            ) == 0
        ) {
            throw valgrind_out_format_error(
                    "Wrong format of valgrind result output. "
                    "Unable to read total heap usage."
            );
        }
    }

    static std::string obtain_line_from_string_that_starts_with(
            const std::string& str, const std::string& starts_with
    ) {
        size_t begin = str.find(starts_with);
        size_t end = begin;
        if (begin == std::string::npos) {
            throw std::invalid_argument(
                utils::string_format(
                    "There is no such line that starts with \"%s\" in the string:\n %s",
                    starts_with.c_str(),
                    str.c_str()
                )
            );
        }
        while (str[end] != '\n' && end < str.size()) {
            end++;
        }
        return str.substr(begin, end - begin + 1);
    }

    const inspectors_output output_;
    size_t stack_allocs_{};
    size_t heap_allocs_{};
};

std::ostream& operator<<(std::ostream& os, const parsed_inspectors_output::inspectors_result& res) {
    os << "stack allocations: " << res.stack_allocs << "\n"
       << "heap allocations: " << res.heap_allocs << "\n"
       << "stack allocations fraction: " << res.stack_allocs_fraction << "%" << "\n"
       << "heap allocations fraction: " << res.heap_allocs_fraction << "%";
    return os;
}

}