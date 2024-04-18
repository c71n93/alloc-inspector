import os
import sys
import pandas
from common import HEADER


result_header = [
    "Repository Name", "Num Of Executables", "AVG Heap Allocs Fraction", "AVG Stack Allocs", "SUM Stack Allocs",
    "AVG Heap Allocs", "SUM Heap Allocs"
]


def is_float(element: any) -> bool:
    if element is None:
        return False
    try:
        float(element)
        return True
    except ValueError:
        return False


def process_single_repository_result(result_csv_filename):
    data = pandas.read_csv(result_csv_filename, index_col=HEADER[0])
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


def gather_final_result_for_directory(directory, final_filename, skip=None):
    if skip is None:
        skip = []
    final_result = pandas.DataFrame(columns=result_header)
    for filename in os.listdir(directory):
        if filename not in skip:
            abs_path = os.path.join(directory, filename)
            single_result = pandas.read_csv(abs_path, index_col=HEADER[0])
            final_result.loc[len(final_result.index)] = [
                filename.split(".")[0], len(single_result.index) - 2,
                single_result.loc["AVERAGE"]["Heap Allocs Fraction"],
                single_result.loc["AVERAGE"]["Stack Allocs"],
                single_result.loc["SUM"]["Stack Allocs"],
                single_result.loc["AVERAGE"]["Heap Allocs"],
                single_result.loc["SUM"]["Heap Allocs"]
            ]
    final_result.to_csv(final_filename, index=False)


def gather_all_binaries_in_single_result(directory, final_filename, skip=None):
    if skip is None:
        skip = []
    final_result = pandas.DataFrame(columns=HEADER)
    for filename in os.listdir(directory):
        if filename not in skip:
            abs_path = os.path.join(directory, filename)
            single_result = pandas.read_csv(abs_path, index_col=HEADER[0]).drop(["AVERAGE", "SUM"], errors="ignore")
            if final_result.empty:
                final_result = single_result
            else:
                final_result = pandas.concat([final_result, single_result])
    final_result["Status"] = final_result["Status"].astype("bool")
    final_result.sort_values(by=["Status", "Executable"], ascending=False).to_csv(final_filename)


# tmp
def add_status_to_results(directory, skip=None):
    if skip is None:
        skip = []
    for filename in os.listdir(directory):
        if filename not in skip:
            abs_path = os.path.join(directory, filename)
            single_result = pandas.read_csv(abs_path, index_col=HEADER[0])
            for index, row in single_result.drop(["AVERAGE", "SUM"], errors="ignore").iterrows():
                if is_float(row["Heap Allocs Fraction"]):
                    single_result.loc[index, "Status"] = True
                else:
                    single_result.loc[index, "Status"] = False
            single_result.sort_values(by="Status").to_csv(abs_path)


def main() -> int:
    skip = ["final-old.csv", "final-c.csv", "final-c++.csv", "summary-c.csv", "summary-c++.csv"]
    add_status_to_results("./results/c", skip)
    add_status_to_results("./results/c++", skip)
    process_all_repositories_results_from_directory("./results/c", skip=skip)
    process_all_repositories_results_from_directory("./results/c++", skip=skip)
    gather_final_result_for_directory("./results/c", "final-c.csv", skip=skip)
    gather_final_result_for_directory("./results/c++", "final-c++.csv", skip=skip)
    gather_all_binaries_in_single_result("./results/c", "summary-c.csv", skip=skip)
    gather_all_binaries_in_single_result("./results/c++", "summary-c++.csv", skip=skip)
    print(pandas.read_csv("final-c.csv")["AVG Heap Allocs Fraction"].mean())
    print(pandas.read_csv("final-c++.csv")["AVG Heap Allocs Fraction"].mean())
    return 0


if __name__ == '__main__':
    sys.exit(main())
