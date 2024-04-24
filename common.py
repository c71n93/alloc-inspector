HEADER = [
    "Executable", "Stack Allocs", "Stack Inspector Runs", "Heap Allocs", "Heap Frees", "Summary Bytes Allocated",
    "Average Bytes Per Allocation", "Valgrind Runs", "Valgrind Error Summary", "Stack Allocs Fraction",
    "Heap Allocs Fraction", "Executable Size", "Elapsed Time", "Status", "Reason"
]


def column_to_bool(dataframe, column_name):
    for index, row in dataframe.iterrows():
        dataframe.loc[index, column_name] = True if str(dataframe.loc[index, column_name]) == "True" else False
    dataframe[column_name] = dataframe[column_name].astype(bool)


def filter_errors(dataframe):
    column_to_bool(dataframe, "Status")
    return dataframe[dataframe["Status"] == True]
