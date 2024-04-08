#pragma once

#include <cstdio>
#include <stdexcept>
#include <sstream>
#include <algorithm>
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
        struct valgrind_result {
            explicit
            valgrind_result(size_t heap_allocs = 0, size_t heap_frees = 0, size_t heap_memory = 0,
                            size_t errors = 0)
                : heap_allocs(heap_allocs), heap_frees(heap_frees), heap_memory(heap_memory),
                errors(errors) {}

            const size_t heap_allocs;
            const size_t heap_frees;
            const size_t heap_memory;
            const double avg_heap_alloc_sz = heap_memory == 0? 0 :
                static_cast<double>(heap_memory) / static_cast<double>(heap_allocs);
            const size_t errors;
        };

        explicit inspectors_result(
            size_t stack_allocs = 0,
            valgrind_result valgrind = valgrind_result{}
        ) : stack_allocs(stack_allocs), valgrind(valgrind) {}

        const size_t stack_allocs = 0;
        const valgrind_result valgrind{};
        const double stack_allocs_fraction = stack_allocs + valgrind.heap_allocs == 0 ? 0 :
            static_cast<double>(stack_allocs) /
            static_cast<double>(stack_allocs + valgrind.heap_allocs);
        const double heap_allocs_fraction = stack_allocs + valgrind.heap_allocs == 0 ? 0 :
            static_cast<double>(valgrind.heap_allocs) /
            static_cast<double>(stack_allocs + valgrind.heap_allocs);

        std::string as_csv() {
            return utils::string_format(
                "%lu,%lu,%lu,%lu,%.17g,%lu,%.17g,%.17g",
                stack_allocs,
                valgrind.heap_allocs,
                valgrind.heap_frees,
                valgrind.heap_memory,
                valgrind.avg_heap_alloc_sz,
                valgrind.errors,
                stack_allocs_fraction,
                heap_allocs_fraction
            );
        }
    };

    parsed_inspectors_output(
            const std::string& stack_inspector_output, const std::string& valgrind_output
    ) : output_{stack_inspector_output, valgrind_output}, result_(parse_stack_inspector(), parse_valgrind()) {}

    inspectors_result take() {
        return result_;
    }

private:
    size_t parse_stack_inspector() {
        std::string info;
        try {
            info = obtain_last_line_from_string_that_starts_with(
                    output_.stack_inspector_output,
                    "Instrumentation results:"
            );
        } catch (const std::invalid_argument& e) {
            throw stack_inspector_out_format_error(e.what());
        }
        size_t stack_allocs_;
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
        return stack_allocs_;
    }

    inspectors_result::valgrind_result parse_valgrind() {
        std::string info;
        try {
            info = obtain_last_line_from_string_that_starts_with(
                    output_.valgrind_output,
                    "total heap usage:"
            );
        } catch (const std::invalid_argument& e) {
            throw valgrind_out_format_error(e.what());
        }
        // TODO: 19 is hardcode for maximum decimal digits in uint64 number
        std::unique_ptr<char[]> heap_allocs_strbuf(new char[19]);
        std::unique_ptr<char[]> heap_frees_strbuf(new char[19]);
        std::unique_ptr<char[]> heap_memory_strbuf(new char[19]);
        if (
            sscanf(
                info.c_str(),
                "total heap usage: %s allocs, %s frees, %s bytes allocated",
                heap_allocs_strbuf.get(),
                heap_frees_strbuf.get(),
                heap_memory_strbuf.get()
            ) == 0
        ) {
            throw valgrind_out_format_error(
                "Wrong format of valgrind result output. "
                "Unable to read total heap usage."
            );
        }
        std::string error;
        try {
            error = obtain_last_line_from_string_that_starts_with(
                    output_.valgrind_output,
                    "ERROR SUMMARY:"
            );
        } catch (const std::invalid_argument& e) {
            throw valgrind_out_format_error(e.what());
        }
        size_t errors;
        if (sscanf(error.c_str(), "ERROR SUMMARY: %ld errors", &errors) == 0) {
            throw valgrind_out_format_error(
                    "Wrong format of valgrind result output. "
                    "Unable to read ERROR SUMMARY."
            );
        }
        return inspectors_result::valgrind_result{
            comma_number_to_int(heap_allocs_strbuf.get()),
            comma_number_to_int(heap_frees_strbuf.get()),
            comma_number_to_int(heap_memory_strbuf.get()),
            errors
        };
    }

    static std::string obtain_last_line_from_string_that_starts_with(
            const std::string& str, const std::string& starts_with
    ) {
        size_t begin = str.rfind(starts_with);
        size_t end = begin;
        if (begin == std::string::npos) {
            throw std::invalid_argument(
                utils::string_format(
                    "There is no such line that starts with \"%s\" in the given string.",
                    starts_with.c_str()
                )
            );
        }
        while (str[end] != '\n' && end < str.size()) {
            end++;
        }
        return str.substr(begin, end - begin + 1);
    }

    size_t comma_number_to_int(std::string number) {
        number.erase(
            std::remove(number.begin(), number.end(), ','),
            number.end()
        );
        return std::atoi(number.c_str());
    }

    const inspectors_output output_;
    const inspectors_result result_;
};

std::ostream& operator<<(std::ostream& os, const parsed_inspectors_output::inspectors_result& res) {
    os << "stack allocations: " << res.stack_allocs << "\n"
       << "heap allocations: " << res.valgrind.heap_allocs << "\n"
       << "heap frees: " << res.valgrind.heap_frees << "\n"
       << "heap bytes allocated: " << res.valgrind.heap_memory << "\n"
       << "heap average allocation size: " << res.valgrind.avg_heap_alloc_sz << "\n"
       << "valgrind error summary: " << res.valgrind.errors << "\n"
       << "stack allocations fraction: " << res.stack_allocs_fraction * 100 << "%" << "\n"
       << "heap allocations fraction: " << res.heap_allocs_fraction * 100 << "%";
    return os;
}

}