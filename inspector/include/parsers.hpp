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
        struct stack_inspector_results {
            explicit
            stack_inspector_results() : stack_allocs(0), num_of_processes(0) {}
            explicit
            stack_inspector_results(size_t stack_allocs, size_t num_of_runs)
                : stack_allocs(stack_allocs), num_of_processes(num_of_runs) {}

            const size_t stack_allocs;
            const size_t num_of_processes;
        };
        struct valgrind_result {
            valgrind_result() : heap_allocs(0), heap_frees(0), heap_memory(0), num_of_processes(0),
                                errors(0) {}
            explicit
            valgrind_result(size_t heap_allocs, size_t heap_frees, size_t heap_memory,
                            size_t num_of_processes, size_t errors)
                : heap_allocs(heap_allocs), heap_frees(heap_frees), heap_memory(heap_memory),
                  num_of_processes(num_of_processes), errors(errors) {}

            const size_t heap_allocs;
            const size_t heap_frees;
            const size_t heap_memory;
            const double avg_heap_alloc_sz = heap_memory == 0? 0 :
                static_cast<double>(heap_memory) / static_cast<double>(heap_allocs);
            const size_t num_of_processes;
            const size_t errors;
        };

        explicit inspectors_result(
            stack_inspector_results stack_inspector,
            valgrind_result valgrind
        ) : stack_inspector{stack_inspector}, valgrind(valgrind) {}

        const stack_inspector_results stack_inspector{};
        const valgrind_result valgrind{};
        const double stack_allocs_fraction = stack_inspector.stack_allocs + valgrind.heap_allocs == 0 ? 0 :
            static_cast<double>(stack_inspector.stack_allocs) /
            static_cast<double>(stack_inspector.stack_allocs + valgrind.heap_allocs);
        const double heap_allocs_fraction = stack_inspector.stack_allocs + valgrind.heap_allocs == 0 ? 0 :
            static_cast<double>(valgrind.heap_allocs) /
            static_cast<double>(stack_inspector.stack_allocs + valgrind.heap_allocs);

        std::string as_csv() {
            return utils::string_format(
                "%lu,%lu,%lu,%lu,%lu,%.17g,%lu,%lu,%.17g,%.17g",
                stack_inspector.stack_allocs,
                stack_inspector.num_of_processes,
                valgrind.heap_allocs,
                valgrind.heap_frees,
                valgrind.heap_memory,
                valgrind.avg_heap_alloc_sz,
                valgrind.num_of_processes,
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
    //TODO: refactor parsing functoins
    inspectors_result::stack_inspector_results parse_stack_inspector() {
        size_t occurrences = 0;
        size_t summary_allocs = 0;
        size_t pos = 0;
        std::string info;
        while ((pos = output_.stack_inspector_output.find("Instrumentation results:", pos)) != std::string::npos) {
            info = obtain_next_line(pos, output_.stack_inspector_output);
            ++occurrences;
            summary_allocs += obtain_single_stack_inspector_result(info);
            pos += info.length();
        }
        if (occurrences == 0) {
            throw stack_inspector_out_format_error(
                utils::string_format(
                    "There is no \"Instrumentation results:\" string in stack_inspector output:\n %s",
                    output_.stack_inspector_output.c_str()
                )
            );
        }
        return inspectors_result::stack_inspector_results{
            summary_allocs, occurrences
        };
    }
    static size_t obtain_single_stack_inspector_result(const std::string& result) {
        size_t stack_allocs;
        if (
            sscanf(
                result.c_str(),
                "Instrumentation results: %lu", &stack_allocs
            ) == 0
        ) {
            throw stack_inspector_out_format_error(
                "Wrong format of stack_inspector result output. "
                "Unable to read instrumentation result."
            );
        }
        return stack_allocs;
    }

    inspectors_result::valgrind_result parse_valgrind() {
        size_t occurrences = 0;
        size_t heap_allocs_summary = 0;
        size_t heap_frees_summary = 0;
        size_t heap_memory_summary = 0;
        size_t pos = 0;
        std::string info;
        while ((pos = output_.valgrind_output.find("total heap usage:", pos)) != std::string::npos) {
            info = obtain_next_line(pos, output_.valgrind_output);
            ++occurrences;
            inspectors_result::valgrind_result single_res = obtain_single_valgrind_result(info);
            heap_allocs_summary += single_res.heap_allocs;
            heap_frees_summary += single_res.heap_frees;
            heap_memory_summary += single_res.heap_memory;
            pos += info.length();
        }
        //TODO: sum up errors too
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
            heap_allocs_summary,
            heap_frees_summary,
            heap_memory_summary,
            occurrences,
            errors
        };
    }
    static inspectors_result::valgrind_result obtain_single_valgrind_result(const std::string& result) {
        // TODO: 19 is hardcode for maximum decimal digits in uint64 number
        std::unique_ptr<char[]> heap_allocs_strbuf(new char[19]);
        std::unique_ptr<char[]> heap_frees_strbuf(new char[19]);
        std::unique_ptr<char[]> heap_memory_strbuf(new char[19]);
        if (
            sscanf(
                result.c_str(),
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
        return inspectors_result::valgrind_result{
                comma_number_to_int(heap_allocs_strbuf.get()),
                comma_number_to_int(heap_frees_strbuf.get()),
                comma_number_to_int(heap_memory_strbuf.get()),
                0,
                0
        };
    }

    static std::string obtain_last_line_from_string_that_starts_with(
            const std::string& str, const std::string& starts_with
    ) {
        size_t begin = str.rfind(starts_with);
        if (begin == std::string::npos) {
            throw std::invalid_argument(
                utils::string_format(
                    "There is no such line that starts with \"%s\" in the string:\n \"%s\".",
                    starts_with.c_str(),
                    str.c_str()
                )
            );
        }
        return obtain_next_line(begin, str);
    }
    static std::string obtain_next_line(size_t begin, const std::string& str) {
        size_t end = begin;
        while (str[end] != '\n' && end < str.size()) {
            end++;
        }
        return str.substr(begin, end - begin + 1);
    }
    static size_t comma_number_to_int(std::string number) {
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
    os << "stack allocations: " << res.stack_inspector.stack_allocs << "\n"
       << "stack_inspector runs: " << res.stack_inspector.num_of_processes
       << "heap allocations: " << res.valgrind.heap_allocs << "\n"
       << "heap frees: " << res.valgrind.heap_frees << "\n"
       << "heap bytes allocated: " << res.valgrind.heap_memory << "\n"
       << "heap average allocation size: " << res.valgrind.avg_heap_alloc_sz << "\n"
       << "valgrind runs: " << res.valgrind.num_of_processes << "\n"
       << "valgrind error summary: " << res.valgrind.errors << "\n"
       << "stack allocations fraction: " << res.stack_allocs_fraction * 100 << "%" << "\n"
       << "heap allocations fraction: " << res.heap_allocs_fraction * 100 << "%";
    return os;
}

}