import os
import sys
import pandas
import matplotlib.pyplot as plt
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


def find_avg_and_sum_for_single_result(result_csv_filename):
    data = pandas.read_csv(result_csv_filename, index_col=HEADER[0])
    data_clean = data.drop(["AVERAGE", "SUM"], errors="ignore").drop(columns=["Reason"], errors="ignore")
    data_without_errors = filter_errors(data_clean)
    data.loc["AVERAGE"] = data_without_errors.mean()
    data.loc["SUM"] = data_without_errors.sum()
    data.to_csv(result_csv_filename)


def find_avg_and_sum_for_all_results_from_directory(directory, skip=None):
    if skip is None:
        skip = []
    for filename in os.listdir(directory):
        if filename not in skip:
            abs_path = os.path.join(directory, filename)
            find_avg_and_sum_for_single_result(abs_path)


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
    column_to_bool(final_result, "Status")
    final_result.sort_values(by=["Status", "Executable"], ascending=False).to_csv(final_filename)


def combine_c_and_cpp_summary_results(c_res_file, cpp_res_file, res_file):
    c_dataframe = pandas.read_csv(c_res_file, index_col=HEADER[0])
    c_dataframe["Language"] = "C"
    cpp_dataframe = pandas.read_csv(cpp_res_file, index_col=HEADER[0])
    cpp_dataframe["Language"] = "C++"
    pandas.concat([cpp_dataframe, c_dataframe]).to_csv(res_file)


def combine_c_and_cpp_repo_results(c_res_file, cpp_res_file, res_file):
    c_dataframe = pandas.read_csv(c_res_file, index_col="Repository Name")
    c_dataframe["Language"] = "C"
    cpp_dataframe = pandas.read_csv(cpp_res_file, index_col="Repository Name")
    cpp_dataframe["Language"] = "C++"
    pandas.concat([cpp_dataframe, c_dataframe]).to_csv(res_file)


def column_to_bool(dataframe, column_name):
    for index, row in dataframe.iterrows():
        dataframe.loc[index, column_name] = True if str(dataframe.loc[index, column_name]) == "True" else False
    dataframe[column_name] = dataframe[column_name].astype(bool)


def find_correlations(result_csv_filename):
    result = pandas.read_csv(
        result_csv_filename,
        index_col=HEADER[0]
    ).drop(["AVERAGE", "SUM"], errors="ignore")
    result_no_errors = filter_errors(result)
    heap_fraction = "Heap Allocs Fraction"
    heap_allocs = "Heap Allocs"
    stack_allocs = "Stack Allocs"
    executable_size = "Executable Size"
    summary_allocated = "Summary Bytes Allocated"
    print("Correlations for file", result_csv_filename, sep=": ")
    print(
        heap_fraction + " and " + summary_allocated,
        result_no_errors[heap_fraction].corr(result_no_errors[summary_allocated]),
        sep=": "
    )
    print(
        heap_fraction + " and " + executable_size,
        result_no_errors[heap_fraction].corr(result_no_errors[executable_size]),
        sep=": "
    )
    print(
        heap_allocs + " and " + executable_size,
        result_no_errors[heap_allocs].corr(result_no_errors[executable_size]),
        sep=": "
    )
    print(
        stack_allocs + " and " + executable_size,
        result_no_errors[stack_allocs].corr(result_no_errors[executable_size]),
        sep=": "
    )


def plot_histogram_summary(result_csv_filename, save_to_dir, bins=100, threshold=1):
    result = pandas.read_csv(
        result_csv_filename,
        index_col=HEADER[0]
    ).drop(["AVERAGE", "SUM"], errors="ignore")
    result_no_errors = filter_errors(result)
    heap_allocs_fraction = result_no_errors["Heap Allocs Fraction"].to_numpy()
    plot_histogram(heap_allocs_fraction, save_to_dir, bins, threshold)


def plot_histogram_repos(result_csv_filename, save_to_dir, bins=100, threshold=1):
    result = pandas.read_csv(
        result_csv_filename,
    )
    heap_allocs_fraction = result["AVG Heap Allocs Fraction"].to_numpy()
    plot_histogram(heap_allocs_fraction, save_to_dir, bins, threshold)


def plot_histogram(data, save_to_dir, bins=100, threshold=1):
    fig, ax = plt.subplots()
    fig.set_size_inches(12, 8)
    ax.set_xlabel('Value')
    ax.set_ylabel('Number Of Executables')
    ax.set_title(f"Histogram of Heap Allocations Fraction. Number of bins={bins}. Values are less than threshold={threshold}")
    ax.hist(data[data < threshold], bins)
    plt.savefig(save_to_dir + f"/bins{bins}-threshold{threshold}.png")
    plt.clf()
    plt.close(fig)


def boxplot(result_csv_filename):
    result = pandas.read_csv(
        result_csv_filename,
        index_col=HEADER[0]
    ).drop(["AVERAGE", "SUM"], errors="ignore")
    result_no_errors = filter_errors(result)
    heap_allocs_fraction = result_no_errors["Heap Allocs Fraction"].to_numpy()
    plt.boxplot(heap_allocs_fraction)
    plt.show()


def filter_errors(dataframe):
    column_to_bool(dataframe, "Status")
    return dataframe[dataframe["Status"] == True]


def main() -> int:
    skip = ["final-old.csv", "repo-summary-c.csv", "repo-summary-c++.csv", "summary-c.csv", "summary-c++.csv", "s2n-tls-old.csv"]
    # find_avg_and_sum_for_all_results_from_directory("./results/c", skip=skip)
    # find_avg_and_sum_for_all_results_from_directory("./results/c++", skip=skip)
    # gather_final_result_for_directory("./results/c", "repo-summary-c.csv", skip=skip)
    # gather_final_result_for_directory("./results/c++", "repo-summary-c++.csv", skip=skip)
    # gather_all_binaries_in_single_result("./results/c", "summary-c.csv", skip=skip)
    # gather_all_binaries_in_single_result("./results/c++", "summary-c++.csv", skip=skip)
    # combine_c_and_cpp_results("./results/c/summary-c.csv", "results/c++/summary-c++.csv", "results.csv")
    # combine_c_and_cpp_repo_results("./results/c/repo-summary-c.csv", "results/c++/repo-summary-c++.csv", "repo-results.csv")

    # find_correlations("results/c/summary-c.csv")
    # find_correlations("results/c++/summary-c++.csv")
    # find_correlations("results/results.csv")

    # for bins in [10, 50, 100, 150, 200]:
    #     for threshold in [1, 0.2, 0.15, 0.1, 0.05]:
    #         plot_histogram("results/c/summary-c.csv", "/Users/c71n93/Desktop/histograms/C", bins, threshold)
    #         plot_histogram("results/c++/summary-c++.csv", "/Users/c71n93/Desktop/histograms/C++", bins, threshold)
    #         plot_histogram("results/results.csv", "/Users/c71n93/Desktop/histograms/Both", bins, threshold)

    # boxplot("results/c/summary-c.csv")
    # boxplot("results/c++/summary-c++.csv")
    # boxplot("results/results.csv")

    # plot_histogram_repos("results/c/repo-summary-c.csv", "/Users/c71n93/Desktop/hist/c", 10)
    # plot_histogram_repos("results/c++/repo-summary-c++.csv", "/Users/c71n93/Desktop/hist/c++", 10)
    # plot_histogram_repos("results/repo-results.csv", "/Users/c71n93/Desktop/hist/both", 6)

    return 0


if __name__ == '__main__':
    sys.exit(main())
