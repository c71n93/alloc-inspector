#pragma once

#include "subprocess.hpp"
#include "results.hpp"

namespace inspector::valgrind {

namespace sp = subprocess;
using results::ValgrindResults;

class valgrind_error : public std::logic_error {
public:
    valgrind_error(const char* msg) : std::logic_error(msg) {}
    valgrind_error(const std::string& msg) : std::logic_error(msg) {}
    virtual ~valgrind_error() {}
};

class valgrind_timeout_error final : public valgrind_error {
public:
    valgrind_timeout_error(const char* msg) : valgrind_error(msg) {}
    valgrind_timeout_error(const std::string& msg) : valgrind_error(msg) {}
};

class valgrind_out_format_error final : public valgrind_error {
public:
    valgrind_out_format_error(const char* msg) : valgrind_error(msg) {}
    valgrind_out_format_error(const std::string& msg) : valgrind_error(msg) {}
};

class ParsedValgrindOutput final {
public:
    ParsedValgrindOutput(const std::string& valgrind_output) : results_(parseValgrindOutput(valgrind_output)){}

    const ValgrindResults& take() {
        return results_;
    }
private:
    //TODO: refactor parsing
    // TODO: refactor exceptons
    static ValgrindResults parseValgrindOutput(const std::string& valgrind_output) {
        size_t occurrences = 0;
        size_t heap_allocs_summary = 0;
        size_t heap_frees_summary = 0;
        size_t heap_memory_summary = 0;
        size_t pos = 0;
        std::string info;
        while ((pos = valgrind_output.find("total heap usage:", pos)) != std::string::npos) {
            info = util::getThisLineFromString(pos, valgrind_output);
            ++occurrences;
            ValgrindResults single_res{0, 0, 0, 0, 0};
            try {
                single_res = obtain_single_valgrind_result(info);
            } catch (const valgrind_out_format_error& e) {
                throw valgrind_out_format_error(
                    util::stringFormat(
                        "%s. The output was: %s",
                        e.what(),
                        valgrind_output.c_str()
                    )
                );
            }
            heap_allocs_summary += single_res.heap_allocs();
            heap_frees_summary += single_res.heap_frees();
            heap_memory_summary += single_res.heap_memory();
            pos += info.length();
        }
        //TODO: sum up errors too
        std::string error;
        try {
            error = obtain_last_line_from_string_that_starts_with(
                valgrind_output,
                "ERROR SUMMARY:"
            );
        } catch (const std::invalid_argument& e) {
            throw valgrind_out_format_error(e.what());
        }
        size_t errors;
        if (sscanf(error.c_str(), "ERROR SUMMARY: %ld errors", &errors) == 0) {
            throw valgrind_out_format_error(
                util::stringFormat(
                    "Wrong format of valgrind result output. Unable to read ERROR SUMMARY. The output was: %s",
                    valgrind_output.c_str()
                )
            );
        }
        return ValgrindResults{
            heap_allocs_summary,
            heap_frees_summary,
            heap_memory_summary,
            occurrences,
            errors
        };
    }
    static ValgrindResults obtain_single_valgrind_result(const std::string& result) {
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
                "Wrong format of valgrind result output. Unable to read total heap usage."
            );
        }
        return ValgrindResults{
            util::commaNumberToInt(heap_allocs_strbuf.get()),
            util::commaNumberToInt(heap_frees_strbuf.get()),
            util::commaNumberToInt(heap_memory_strbuf.get()),
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
                util::stringFormat(
                    "There is no such line that starts with \"%s\" in the string:\n \"%s\".",
                    starts_with.c_str(),
                    str.c_str()
                )
            );
        }
        return util::getThisLineFromString(begin, str);
    }
private:
    ValgrindResults results_;
};

class ValgrindLauncher final {
public:
    ValgrindLauncher(size_t timelimit, const std::string &executable)
        : timelimit_(timelimit), executable_(executable) {}

    const ValgrindResults &getResults() const {
        return results_;
    }
    const ValgrindResults &launchWithResults() {
        results_ = ParsedValgrindOutput(runValgrind()).take();
        return results_;
    }

private:
    std::string runValgrind() {
        auto valgrind_process = sp::Popen(
            util::stringFormat(
                "timeout --signal=SIGKILL %d valgrind --trace-children=yes %s",
                timelimit_,
                executable_.c_str()
            ),
            sp::error{sp::PIPE}
        );
        std::string valgrind_output{valgrind_process.communicate().second.buf.data()};
        auto valgrind_retcode = valgrind_process.retcode();
        if (valgrind_retcode == SIGKILL) {
            throw valgrind_timeout_error(
                util::stringFormat(
                    "valgrind is killed due to exceeding the time limit of %ds",
                    timelimit_
                )
            );
        }
        return valgrind_output;
    }

private:
    size_t timelimit_;
    std::string executable_;
    ValgrindResults results_{0, 0, 0, 0, 0};
};

}