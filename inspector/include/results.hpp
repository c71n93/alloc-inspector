#pragma once

#include <cstddef>

namespace inspector::results {

class StackInspectorResults final {
public:
    StackInspectorResults(size_t stack_allocs, size_t num_of_runs)
        : stack_allocs_(stack_allocs), num_of_processes_(num_of_runs) {}

    size_t stack_allocs() const {
        return stack_allocs_;
    }
    size_t num_of_processes() const {
        return num_of_processes_;
    }
private:
    size_t stack_allocs_ = 0;
    size_t num_of_processes_ = 0;
};

class ValgrindResults final {
public:
    ValgrindResults(size_t heap_allocs, size_t heap_frees, size_t heap_memory, size_t num_of_processes, size_t errors)
        : heap_allocs_(heap_allocs), heap_frees_(heap_frees), heap_memory_(heap_memory),
          num_of_processes_(num_of_processes), errors_(errors) {}

    size_t heap_allocs() const {
        return heap_allocs_;
    }
    size_t heap_frees() const {
        return heap_frees_;
    }
    size_t heap_memory() const {
        return heap_memory_;
    }
    double avg_heap_alloc_sz() const {
        return avg_heap_alloc_sz_;
    }
    size_t num_of_processes() const {
        return num_of_processes_;
    }
    size_t errors() const {
        return errors_;
    }

private:
    size_t heap_allocs_;
    size_t heap_frees_;
    size_t heap_memory_;
    double avg_heap_alloc_sz_ = heap_memory_ == 0? 0 :
        static_cast<double>(heap_memory_) / static_cast<double>(heap_allocs_);
    size_t num_of_processes_;
    size_t errors_;
};

class InspectorResults final {
public:
    explicit InspectorResults(
        StackInspectorResults stack_inspector,
        ValgrindResults valgrind
    ) : stack_inspector_{stack_inspector}, valgrind_(valgrind) {}

    std::string as_csv() {
        return util::stringFormat(
            "%lu,%lu,%lu,%lu,%lu,%.17g,%lu,%lu,%.17g,%.17g",
            stack_inspector_.stack_allocs(),
            stack_inspector_.num_of_processes(),
            valgrind_.heap_allocs(),
            valgrind_.heap_frees(),
            valgrind_.heap_memory(),
            valgrind_.avg_heap_alloc_sz(),
            valgrind_.num_of_processes(),
            valgrind_.errors(),
            stack_allocs_fraction_,
            heap_allocs_fraction_
        );
    }

    const ValgrindResults& valgrind() const {
        return valgrind_;
    }
    const StackInspectorResults& stack_inspector() const {
        return stack_inspector_;
    }
    double stack_allocs_fraction() const {
        return stack_allocs_fraction_;
    }
    double heap_allocs_fraction() const {
        return heap_allocs_fraction_;
    }

private:
    StackInspectorResults stack_inspector_{0, 0};
    ValgrindResults valgrind_{0, 0, 0, 0, 0};
    double stack_allocs_fraction_ = stack_inspector_.stack_allocs() + valgrind_.heap_allocs() == 0 ? 0 :
        static_cast<double>(stack_inspector_.stack_allocs()) /
        static_cast<double>(stack_inspector_.stack_allocs() + valgrind_.heap_allocs());
    double heap_allocs_fraction_ = stack_inspector_.stack_allocs() + valgrind_.heap_allocs() == 0 ? 0 :
        static_cast<double>(valgrind_.heap_allocs()) /
        static_cast<double>(stack_inspector_.stack_allocs() + valgrind_.heap_allocs());
};

std::ostream& operator<<(std::ostream& os, const StackInspectorResults& res) {
    os << "stack allocations: " << res.stack_allocs() << "\n"
       << "stack_inspector runs: " << res.num_of_processes();
    return os;
}

std::ostream& operator<<(std::ostream& os, const InspectorResults& res) {
    os << res.stack_inspector() << "\n"
       << "heap allocations: " << res.valgrind().heap_allocs() << "\n"
       << "heap frees: " << res.valgrind().heap_frees() << "\n"
       << "heap bytes allocated: " << res.valgrind().heap_memory() << "\n"
       << "heap average allocation size: " << res.valgrind().avg_heap_alloc_sz() << "\n"
       << "valgrind runs: " << res.valgrind().num_of_processes() << "\n"
       << "valgrind error summary: " << res.valgrind().errors() << "\n"
       << "stack allocations fraction: " << res.stack_allocs_fraction() * 100 << "%" << "\n"
       << "heap allocations fraction: " << res.heap_allocs_fraction() * 100 << "%";
    return os;
}

}
