import sys
import os
import subprocess
import csv
from io import StringIO
import time
from common import HEADER


SIZE_OF_INSPECTOR_OUT = 10


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


def name_contains_all_of(name, contains):
    for item in contains:
        if item not in name:
            return False
    return True


def collect_executables_from_directories(directories, accepted_substrings=None, skip=None, ignored_substrings=None,
                                         collect_recursively=True):
    if ignored_substrings is None:
        ignored_substrings = []
    if skip is None:
        skip = []
    paths = []
    for directory in directories:
        for filename in os.listdir(directory):
            abs_path = os.path.join(directory, filename)
            if os.path.isdir(abs_path) and not name_contains_any_of(filename, ignored_substrings):
                if collect_recursively:
                    paths.extend(collect_executables_from_directories([abs_path]))
                else:
                    continue
            elif is_executable(abs_path) and abs_path not in skip and not name_contains_any_of(filename,
                                                                                               ignored_substrings):
                if accepted_substrings is None or name_contains_all_of(filename, accepted_substrings):
                    paths.append(abs_path)
    print(f"found {len(paths)} executables:")
    return paths


def inspect_executables_for_repository(inspector_exec, executables, repository_name, result_directory):
    result_csv_file = open(os.path.join(result_directory, repository_name + ".csv"), "w")
    result_csv_writer = csv.writer(result_csv_file, delimiter=",")
    result_csv_writer.writerow(HEADER)
    retcode_to_error = {
        1: "stack_inspector timeout",
        2: "valgrind timeout",
        3: "stack_inspector parsing error",
        4: "valgrind parsing error",
        "default": "error"
    }
    for executable in executables:
        print(f"inspecting: {executable}:")
        start = time.time()
        inspector_process = subprocess.run(
            [os.path.abspath(inspector_exec), os.path.abspath(executable)],
            capture_output=True
        )
        end = time.time()
        file_size = os.stat(executable).st_size
        elapsed = end - start
        f = StringIO(inspector_process.stdout.decode("utf-8"))
        retcode = inspector_process.returncode
        res = list(csv.reader(f, delimiter=","))
        status = True
        if retcode != 0:
            print(f"error: non zero exit code while inspecting {executable}")
            print(inspector_process.stderr.decode("utf-8"))
            res = [[
                retcode_to_error.copy().get(retcode, retcode_to_error.get("default"))
                for _ in range(SIZE_OF_INSPECTOR_OUT)
            ]]
            status = False
        elif len(res) == 0 or len(res[0]) != 10:
            print(f"error: something went wrong while inspecting {executable}")
            print(inspector_process.stderr.decode("utf-8"))
            res = [[retcode_to_error.copy().get("default") for _ in range(SIZE_OF_INSPECTOR_OUT)]]
            status = False
        row = res[0]
        row.insert(0, executable)
        row.append(str(file_size))
        row.append(str(elapsed))
        row.append(str(status))
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
    # OpenCC
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c++/OpenCC/build/dbg/src"],
            collect_recursively=False,
            accepted_substrings=["Test"]
        ),
        "OpenCC",
        result_directory
    )
    # glog
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c++/glog/build"],
            collect_recursively=False,
            accepted_substrings=["test"]
        ),
        "glog",
        result_directory
    )
    # snappy - not suitable
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c++/snappy/build/snappy_unittest"],
        "snappy",
        result_directory
    )
    # yaml-cpp
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c++/yaml-cpp/build/test/yaml-cpp-tests"],
        "yaml-cpp",
        result_directory
    )
    # magic_enum
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c++/magic_enum/build/test",
             "./repos/c++/magic_enum/build/example"],
            collect_recursively=False
        ),
        "magic_enum",
        result_directory
    )

    # openssl
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/openssl/test"],
            skip=["./repos/c/openssl/test/ecstresstest", "./repos/c/openssl/test/bio_prefix_text"], #  timeout
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
        ["./repos/c/mimalloc/build/mimalloc-test-api-fill",
         "./repos/c/mimalloc/build/mimalloc-test-api",
         "./repos/c/mimalloc/build/mimalloc-test-stress"],
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
            accepted_substrings=["test"],
            skip=["./repos/c/s2n-tls/build/bin/s2n_self_talk_client_hello_cb_test",  # infinite test
                  "./repos/c/s2n-tls/build/bin/s2n_io_test",  # 1800 timeout
                  "./repos/c/s2n-tls/build/bin/s2n_self_talk_session_id_test",  # infinite test
                  ]
        ),
        "s2n-tls",
        result_directory
    )
    # json-c
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/json-c/build/tests"],
            collect_recursively=False
        ),
        "json-c",
        result_directory
    )
    # Collections-C
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/Collections-C/build/examples", "./repos/c/Collections-C/build/test"],
            ignored_substrings=["CMakeFiles"],
        ),
        "Collections-C",
        result_directory
    )
    # fastfetch
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c/fastfetch/build/fastfetch",
         "./repos/c/fastfetch/build/flashfetch"],
        "fastfetch",
        result_directory
    )
    # zlog
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/zlog/build/bin"],
            skip=["./repos/c/zlog/build/bin/zlog-chk-conf"]
        ),
        "zlog",
        result_directory
    )
    # onion
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/onion/build/tests/01-internal"],
            collect_recursively=False
        ),
        "onion",
        result_directory
    )
    # cmark
    inspect_executables_for_repository(
        inspector_exec,
        ["./repos/c/cmark/build/api_test/api_test"],
        "cmark",
        result_directory
    )
    # libsndfile
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/libsndfile/build"],
            collect_recursively=False,
            accepted_substrings=["test"]
        ),
        "libsndfile",
        result_directory
    )
    # zip
    inspect_executables_for_repository(
        inspector_exec,
        collect_executables_from_directories(
            ["./repos/c/zip/build/test"],
            collect_recursively=False,
        ),
        "zip",
        result_directory
    )

    return 0


if __name__ == '__main__':
    sys.exit(main())
