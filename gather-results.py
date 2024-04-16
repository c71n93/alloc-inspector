import os
import sys
import csv
import pandas
from common import header


def process_single_repository_result_manual(result_csv_filename):
    result_csv_file = open(result_csv_filename, "r+")
    next(result_csv_file)  # skip heading
    result_csv_reader = csv.reader(result_csv_file)
    avg: list[float] = [0.0] * (len(header) - 1)
    sum_: list[float] = [0.0] * (len(header) - 1)
    csv_rows = list(result_csv_reader)
    executables_num: int = len(csv_rows)
    if executables_num <= 0:
        raise RuntimeError("number of executables in result csv is 0")
    skip = 0
    for row in csv_rows:
        for i in range(1, len(row)):
            try:
                sum_[i - 1] += float(row[i])
            except ValueError:
                skip += 1
    for i in range(len(sum_)):
        avg[i] = sum_[i] / executables_num
    result_csv_writer = csv.writer(result_csv_file)
    result_csv_writer.writerow(["AVERAGE"] + avg)
    result_csv_writer.writerow(["SUM"] + sum_)
    result_csv_file.close()


def process_single_repository_result(result_csv_filename):
    data = pandas.read_csv(result_csv_filename, index_col=header[0])
    data_dropped = data.drop(["AVERAGE", "SUM"], errors="ignore")
    data.loc["AVERAGE"] = data_dropped.mean()
    data.loc["SUM"] = data_dropped.sum()
    data.to_csv(result_csv_filename)


def process_all_repositories_results_from_directory(directory):
    for filename in os.listdir(directory):
        abs_path = os.path.join(directory, filename)
        process_single_repository_result(abs_path)


def main() -> int:
    process_single_repository_result("./results/c/jq.csv")
    # process_all_repositories_results_from_directory("./results/c")
    return 0


if __name__ == '__main__':
    sys.exit(main())
