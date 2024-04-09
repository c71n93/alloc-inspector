import sys
import os
import subprocess
import csv
from io import StringIO
import time
import copy


def is_executable(file):
    process = subprocess.run(["file", file], capture_output=True)
    output = process.stdout.decode("utf-8")
    if "ELF 64-bit" in output and "executable" in output:
        return True
    else:
        return False


def name_contains_any_of(name, contains):
    for item in contains:
        if item in name:
            return True
    return False


def collect_executables_from_directories_recursively(directories, skip=None, ignored_substrings=None):
    if ignored_substrings is None:
        ignored_substrings = []
    if skip is None:
        skip = []
    paths = []
    for directory in directories:
        for filename in os.listdir(directory):
            abs_path = os.path.join(directory, filename)
            if os.path.isdir(abs_path):
                paths.extend(collect_executables_from_directories_recursively([abs_path]))
            elif is_executable(abs_path):
                if abs_path not in skip and not name_contains_any_of(filename, ignored_substrings):
                    paths.append(abs_path)
    print(f"found {len(paths)} executables")
    return paths


def inspect_executables_for_repository(inspector_exec, executables, repository_name, result_directory):
    result_csv_file = open(os.path.join(result_directory, repository_name + ".csv"), "w")
    result_csv_writer = csv.writer(result_csv_file, delimiter=",")
    header = ["Executable", "Stack Allocs", "Stack Inspector Runs", "Heap Allocs", "Heap Frees",
              "Summary Bytes Allocated", "Average Bytes Per Allocation", "Valgrind Runs", "Valgrind Error Summary",
              "Stack Allocs Fraction", "Heap Allocs Fraction", "Executable Size", "Elapsed Time"]
    result_csv_writer.writerow(header)
    retcode_to_error = {
        1: [["stack_inspector timeout" for _ in range(0, len(header) - 3)]],
        2: [["valgrind timeout" for _ in range(0, len(header) - 3)]],
        3: [["stack_inspector parsing error" for _ in range(0, len(header) - 3)]],
        4: [["valgrind parsing error" for _ in range(0, len(header) - 3)]],
        "default": [["error" for _ in range(0, len(header) - 3)]]
    }
    for executable in executables:
        print(f"inspecting: {executable}:")
        start = time.time()
        inspector_process = subprocess.run([os.path.abspath(inspector_exec), executable], capture_output=True)
        end = time.time()
        file_size = os.stat(executable).st_size
        elapsed = end - start
        f = StringIO(inspector_process.stdout.decode("utf-8"))
        retcode = inspector_process.returncode
        res = list(csv.reader(f, delimiter=","))
        if retcode != 0:
            print(f"error: non zero exit code while inspecting {executable}")
            print(inspector_process.stderr.decode("utf-8"))
            res = copy.deepcopy(retcode_to_error.copy().get(retcode, retcode_to_error.get("default")))
        elif len(res) == 0:
            print(f"error: something went wrong while inspecting {executable}")
            print(inspector_process.stderr.decode("utf-8"))
            res = copy.deepcopy(retcode_to_error.get("default").copy())
        row = res[0]
        row.insert(0, executable)
        row.append(str(file_size))
        row.append(str(elapsed))
        print(row[1:len(row)])
        result_csv_writer.writerow(row)
    result_csv_file.close()


def main() -> int:
    inspector_exec = "./inspector/build/alloc_inspector"
    if len(sys.argv) != 2:
        raise RuntimeError("wrong arguments: path to result file is required")
    result_directory = sys.argv[1]
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories_recursively(
            ["path/to/exmaple/executables"],
            skip=["path/to/skip"],
            ignored_substrings=["debug"]
        ),
        "example",
        result_directory
    )

    return 0


if __name__ == '__main__':
    sys.exit(main())
