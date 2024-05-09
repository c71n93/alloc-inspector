import os
import sys
from utils.test_util import parse_args, run_stack_inspector, parse_result, assert_equals


def main() -> int:
    drrun_path, libstack_inspector_path, executable_path = parse_args(sys.argv)
    tmp_filepath = os.getcwd() + "/tmp.txt"
    run_stack_inspector(drrun_path, libstack_inspector_path, executable_path, tmp_filepath)
    result = parse_result(tmp_filepath)
    assert_equals(result, 8 + 22)
    os.remove(tmp_filepath)
    return 0


if __name__ == '__main__':
    sys.exit(main())
