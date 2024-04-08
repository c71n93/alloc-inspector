import sys
import os
import subprocess
import csv
from io import StringIO
import time


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
    print(f"found {len(paths)} executables:")
    print(paths)
    return paths


def inspect_executables_for_repository(inspector_exec, executables, repository_name, result_directory):
    result_csv_file = open(os.path.join(result_directory, repository_name + ".csv"), "w")
    result_csv_writer = csv.writer(result_csv_file, delimiter=",")
    result_csv_writer.writerow(
        ["Executable", "Stack Allocs", "Heap Allocs", "Heap Frees", "Summary Bytes Allocated",
         "Average Bytes Per Allocation", "Stack Allocs Fraction", "Heap Allocs Fraction", "Executable Size",
         "Elapsed Time"]
    )
    for executable in executables:
        print(f"inspecting: {executable}:")
        start = time.time()
        inspector_process = subprocess.run([os.path.abspath(inspector_exec), executable], capture_output=True)
        end = time.time()
        file_size = os.stat(executable).st_size
        elapsed = end - start
        f = StringIO(inspector_process.stdout.decode("utf-8"))
        res = list(csv.reader(f, delimiter=","))
        if len(res) == 0:
            print(f"error: something went wrong while inspecting {executable}")
            print(inspector_process.stderr.decode("utf-8"))
            res = [["error" for _ in range(0, 7)]]
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
