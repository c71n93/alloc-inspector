#pragma once

#include "subprocess.hpp"
#include "results.hpp"
#include "util.hpp"

namespace inspector::stack_inspector {

namespace sp = subprocess;
using results::StackInspectorResults;

class stack_inspector_error : public std::logic_error {
public:
    stack_inspector_error(const char* msg) : std::logic_error(msg) {}
    stack_inspector_error(const std::string& msg) : std::logic_error(msg) {}
    virtual ~stack_inspector_error() {}
};

class stack_inspector_out_format_error final : public stack_inspector_error {
public:
    stack_inspector_out_format_error(const char* msg) : stack_inspector_error(msg) {}
    stack_inspector_out_format_error(const std::string& msg) : stack_inspector_error(msg) {}
};

class stack_inspector_timeout_error final : public stack_inspector_error {
public:
    stack_inspector_timeout_error(const char* msg) : stack_inspector_error(msg) {}
    stack_inspector_timeout_error(const std::string& msg) : stack_inspector_error(msg) {}
};

class ParsedStackInspectorOutput final {
public:
    ParsedStackInspectorOutput(const std::string& stack_inspector_output) :
        results_{parseStackInspectorOutput(stack_inspector_output)} {}

    const StackInspectorResults& take() {
        return results_;
    }
private:
    // TODO: refactor parsing
    // TODO: refactor exceptions
    StackInspectorResults parseStackInspectorOutput(const std::string &stack_inspector_output) {
        size_t occurrences = 0;
        size_t summary_allocs = 0;
        size_t pos = 0;
        std::string info;
        while ((pos = stack_inspector_output.find("Instrumentation results:", pos)) != std::string::npos) {
            info = util::getThisLineFromString(pos, stack_inspector_output);
            ++occurrences;
            try {
                summary_allocs += parseSingleStackInspectorResult(info);
            } catch (const stack_inspector_out_format_error& e) {
                throw stack_inspector_out_format_error(
                    util::stringFormat(
                        "%s. The output was: %s",
                        e.what(),
                        stack_inspector_output.c_str()
                    )
                );
            }
            pos += info.length();
        }
        if (occurrences == 0) {
            throw stack_inspector_out_format_error(
                util::stringFormat(
                    "There is no \"Instrumentation results:\" string in stack_inspector output:\n %s",
                    stack_inspector_output.c_str()
                )
            );
        }
        return StackInspectorResults{summary_allocs, occurrences};
    }
    static size_t parseSingleStackInspectorResult(const std::string& result) {
        size_t stack_allocs;
        if (
            sscanf(
                result.c_str(),
                "Instrumentation results: %lu", &stack_allocs
            ) == 0
            ) {
            throw stack_inspector_out_format_error(
                "Wrong format of stack_inspector result output. Unable to read instrumentation result."
            );
        }
        return stack_allocs;
    }

private:
    StackInspectorResults results_;
};

class StackInspectorLauncher final {
public:
    StackInspectorLauncher(size_t timelimit, const std::string &stack_inspector_exec, const std::string &executable)
        : timelimit_(timelimit), stack_inspector_exec_(stack_inspector_exec), executable_(executable) {}

    const StackInspectorResults &getResults() const {
        return results_;
    }
    const StackInspectorResults &launchWithResults() {
        results_ = ParsedStackInspectorOutput(runStackInspector()).take();
        return results_;
    }

private:
    std::string runStackInspector() {
        auto stack_inspector_process = sp::Popen(
            util::stringFormat(
                "timeout --signal=SIGKILL %d %s %s",
                timelimit_,
                stack_inspector_exec_.c_str(),
                executable_.c_str()
            ),
            sp::output{sp::PIPE}
        );
        std::string stack_inspector_output{stack_inspector_process.communicate().first.buf.data()};
        int stack_inspector_retcode = stack_inspector_process.retcode();
        if (stack_inspector_retcode == SIGKILL) {
            throw stack_inspector_timeout_error(
                util::stringFormat(
                    "stack_inspector is killed due to exceeding the time limit of %ds",
                    timelimit_
                )
            );
        }
        return stack_inspector_output;
    }

private:
    size_t timelimit_;
    std::string stack_inspector_exec_;
    std::string executable_;
    StackInspectorResults results_{0, 0};
};

}