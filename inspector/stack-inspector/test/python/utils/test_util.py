import subprocess


def parse_args(argv):
    if len(argv) != 3:
        raise RuntimeError("wrong arguments: path to stack_inspector_exec and path to testing executable are required")
    stack_inspector_exec = argv[1]
    executable_path = argv[2]
    return stack_inspector_exec, executable_path


def assert_equals(actual, expected):
    if actual != expected:
        raise RuntimeError(f"assertion failed:\n"
                           f"   expected: \"{expected}\";\n"
                           f"   actual: \"{actual}\"")
    else:
        return


def run_stack_inspector(stack_inspector_exec, executable_path, out_filepath):
    cmd = [stack_inspector_exec, executable_path]
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
    assert_equals(end, "stack allocation instructions executed.\n")
    return result
