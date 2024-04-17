import os
import sys
import pandas
from common import header


def process_single_repository_result(result_csv_filename):
    data = pandas.read_csv(result_csv_filename, index_col=header[0])
    data_clean = data.drop(["AVERAGE", "SUM"], errors="ignore").apply(pandas.to_numeric, errors="coerce")
    data.loc["AVERAGE"] = data_clean.mean()
    data.loc["SUM"] = data_clean.sum()
    data.to_csv(result_csv_filename)


def process_all_repositories_results_from_directory(directory, skip=None):
    if skip is None:
        skip = []
    for filename in os.listdir(directory):
        if filename not in skip:
            abs_path = os.path.join(directory, filename)
            process_single_repository_result(abs_path)


def main() -> int:
    process_all_repositories_results_from_directory("./results/c", skip=["final.csv"])
    process_all_repositories_results_from_directory("./results/c++", skip=["final.csv"])
    return 0


if __name__ == '__main__':
    sys.exit(main())
