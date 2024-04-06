import sys
import os
import subprocess
import csv
from io import StringIO
import time


def collect_binaries_from_directories_recursively(directories):
    paths = []
    for directory in directories:
        for filename in os.listdir(directory):
            abs_path = os.path.join(directory, filename)
            if os.path.isdir(abs_path):
                paths.extend(collect_binaries_from_directories_recursively([abs_path]))
            elif "." not in filename and "test" in filename:
                paths.append(abs_path)
                continue
            else:
                continue
    print(paths)
    return paths


def inspect_binaries_for_repository(inspector_exec, binaries, repository_name, result_directory):
    result_csv_file = open(os.path.join(result_directory, repository_name + ".csv"), "w")
    result_csv_writer = csv.writer(result_csv_file, delimiter=",")
    result_csv_writer.writerow(
        ["Executable", "Stack Allocs", "Heap Allocs", "Heap Frees", "Summary Bytes Allocated",
         "Average Bytes Per Allocation", "Stack Allocs Fraction", "Heap Allocs Fraction", "Executable Size",
         "Elapsed Time"]
    )
    for binary in binaries:
        print(f"inspecting: {binary}:")
        start = time.time()
        inspector_process = subprocess.run([os.path.abspath(inspector_exec), binary], capture_output=True)
        end = time.time()
        file_size = os.stat(binary).st_size
        elapsed = end - start
        f = StringIO(inspector_process.stdout.decode("utf-8"))
        res = list(csv.reader(f, delimiter=","))
        if len(res) == 0:
            print(f"error: something went wrong while inspecting {binary}")
            print(inspector_process.stderr.decode("utf-8"))
            res = [["error" for _ in range(0, 7)]]
        row = res[0]
        row.insert(0, binary)
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
    inspect_binaries_for_repository(
        inspector_exec,
        collect_binaries_from_directories_recursively(["/path/to/example"]),
        "example",
        result_directory
    )
    return 0


if __name__ == '__main__':
    sys.exit(main())
