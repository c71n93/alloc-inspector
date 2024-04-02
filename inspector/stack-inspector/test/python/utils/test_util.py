import subprocess


def parse_args(argv):
    if len(argv) != 4:
        raise RuntimeError("wrong arguments: path to ddrun, path to libstack_inspector.so and path to testing "
                           "executable are required")
    ddrun_path = argv[1]
    libstack_inspector_path = argv[2]
    executable_path = argv[3]
    return ddrun_path, libstack_inspector_path, executable_path


def assert_equals(actual, expected):
    if actual != expected:
        raise RuntimeError(f"assertion failed:\n"
                           f"   expected: \"{expected}\";\n"
                           f"   actual: \"{actual}\"")
    else:
        return


def run_stack_inspector(ddrun_path, libstack_inspector_path, executable_path, out_filepath):
    cmd = [ddrun_path, "-c", libstack_inspector_path, "-only_from_app", "--", f"{executable_path}"]
    with open(out_filepath, "w") as out:
        subprocess.run(cmd, stdout=out)


def parse_result(result_filepath) -> int:
    file = open(result_filepath, "r")
    full_string_split = file.readline().split(": ")
    begin = full_string_split[0]
    res_string_split = full_string_split[1].split(" ")
    result = int(res_string_split[0])
    end = " ".join(res_string_split[1:len(res_string_split)])
    assert_equals(begin, "Instrumentation results")
    assert_equals(end, "stack allocation instructions executed\n")
    return result
