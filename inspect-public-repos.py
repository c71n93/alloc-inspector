import sys
import os
import subprocess
import csv
from io import StringIO
import time
import copy
import re


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


def collect_executables_from_directories(directories, accepted_substring=None, skip=None, ignored_substrings=None,
                                         collect_recursively=True):
    if ignored_substrings is None:
        ignored_substrings = []
    if skip is None:
        skip = []
    paths = []
    for directory in directories:
        for filename in os.listdir(directory):
            abs_path = os.path.join(directory, filename)
            if os.path.isdir(abs_path):
                if collect_recursively:
                    paths.extend(collect_executables_from_directories([abs_path]))
                else:
                    continue
            elif is_executable(abs_path) and abs_path not in skip and not name_contains_any_of(filename, ignored_substrings):
                if accepted_substring is None or name_contains_any_of(filename, accepted_substring):
                    paths.append(abs_path)
    print(f"found {len(paths)} executables:")
    print(paths)
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
    # fmt
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c++/fmt/build/bin"],
        ),
        "fmt",
        result_directory
    )
    # spdlog
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c++/spdlog/build/tests/spdlog-utests",
         "./repos/c++/spdlog/build/example/example"],
        "spdlog",
        result_directory
    )
    # leveldb
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c++/leveldb/build/db_bench",
         "./repos/c++/leveldb/build/c_test",
         "./repos/c++/leveldb/build/env_posix_test"],
        "leveldb",
        result_directory
    )
    # json
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c++/json/build/tests"],
            collect_recursively=False
        ),
        "json",
        result_directory
    )
    # MyTinySTL - unexpected valgrind error (mb timeout)
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c++/MyTinySTL/bin/stltest"],
        "MyTinySTL",
        result_directory
    )
    # benchmark
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c++/benchmark/build/test"],
            collect_recursively=False
        ),
        "benchmark",
        result_directory
    )

    # openssl
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/openssl/test"],
            skip=["./repos/c/openssl/test/ecstresstest"],
        ),
        "openssl",
        result_directory
    )
    # dynamorio
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/dynamorio/build/suite/tests/bin/"],
            ignored_substrings=[".debug"]
        ),
        "dynamorio",
        result_directory
    )
    # jq
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c/jq/tests/1_run_mantest",
         "./repos/c/jq/tests/1_run_jqtest"],
        "jq",
        result_directory
    )
    # cJSON
    cJSON_execs = collect_executables_from_directories(
        ["./repos/c/cJSON/build/tests"]
    )
    cJSON_execs.append("./repos/c/cJSON/build/cJSON_test")
    inspect_executables_for_repository(
        inspector_exec,
        cJSON_execs,
        "cJSON",
        result_directory
    )
    # mimalloc
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c/mimalloc/out/release/mimalloc-test-api-fill",
         "./repos/c/mimalloc/out/release/mimalloc-test-api",
         "./repos/c/mimalloc/out/release/mimalloc-test-stress"],
        "mimalloc",
        result_directory
    )
    # nanomsg
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/nanomsg/build"],
            collect_recursively=False
        ),
        "nanomsg",
        result_directory
    )
    # s2n-tls
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/s2n-tls/build/bin"],
            accepted_substring=["test"],
            skip=["./repos/c/s2n-tls/build/bin/s2n_self_talk_client_hello_cb_test"]
        ),
        "s2n-tls",
        result_directory
    )
    # libuv - 1200 timeout
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/libuv/build"],
            collect_recursively=False
        ),
        "libuv",
        result_directory
    )
    # libevent - 1200 timeout
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/libevent/build/bin"],
            skip=["./repos/c/libevent/build/bin/dns-example",
                  "./repos/c/libevent/build/bin/event-read-fifo",
                  "./repos/c/libevent/build/bin/hello-world",
                  "./repos/c/libevent/build/bin/http-connect",
                  "./repos/c/libevent/build/bin/http-server",
                  "./repos/c/libevent/build/bin/watch-timing",
                  "./repos/c/libevent/build/bin/ws-chat-server"
                  ]
        ),
        "libevent",
        result_directory
    )
    return 0


if __name__ == '__main__':
    sys.exit(main())
