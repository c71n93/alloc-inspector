# Alloc Inspector
Tool for dynamically inspecting number of stack and heap allocations executable files.

## Requirements

The following dependencies must be installed:
- [Valgrind](https://valgrind.org/) command line tool. Сan be installed using your package manager, for example:
```console
$ sudo apt-get install valgrind
```
- [DynamoRIO](https://github.com/DynamoRIO/dynamorio) tool. Should be installed from sources
([`stack-inspector`](./inspector/stack-inspector) depends on it). Check 
[this guide](https://dynamorio.org/page_building.html)

## Installation

#### Build
```console
$ git clone https://github.com/c71n93/alloc-inspector
$ cd alloc-inspector/inspector && mkdir build && cd build
$ cmake -DDYNAMORIO_HOME=<dynamorio-build-directory> ..
$ make
```

`DYNAMORIO_HOME` is a path to DynamoRIO build directory. For example, in
[this guide](https://dynamorio.org/page_building.html) it will be `dynamorio/build`.

#### Run Tests (optional)
```console
$ cd alloc-inspector/inspector/build
$ ctest
```

## Usage

#### `stack_inspector_exec`

This tool will show number of stack allocations in your executable:
```console
$ inspector/build/stack-inspector/stack_inspector_exec <some-executable>

TODO
```
#### `alloc_inspector`
This tool will run `stack_inspector` and `valgrind` to show summary information about stack and heap allocations:
```console
$ inspector/build/stack-inspector/stack_inspector_exec <some-executable>

TODO
```

#### Scripts
This tool was used to collect information about stack and heap allocations from public repositories (check 
[Results](#results)). Therefore, there are two scripts for collecting and processing data from a large number of 
binaries:

- [inspect-public-repos.py](inspect-public-repos.py) - contains functions for inspecting built repositories
- [process-results.py](inspect-public-repos.py) - contains function to process data obtained from previous script

## Results

Results can be found in [./results](./results) in csv format.

### Inspected Repositories

To reproduce the results, you need to clone projects into the `./repos/c` and `./repos/c++` directories for C and C++
projects respectfully. At the next step projects should be built according to the README for each project (use build 
with tests). Finally, [inspect-public-repos.py](./inspect-public-repos.py) can be run to collect results in CSV format
for every repository.

#### C
| Repository                                              | Commit                                   |
|---------------------------------------------------------|------------------------------------------|
| [cJSON](https://github.com/DaveGamble/cJSON)            | 87d8f0961a01bf09bef98ff89bae9fdec42181ee |
| [cmark](https://github.com/commonmark/cmark)            | 2632fdce27d312a6b0a0b506e35c8270d7571781 |
| [Collections-C](https://github.com/srdja/Collections-C) | 6b6ec211afa5fc21ede02607cf227a7a6bee7bac |
| [fastfetch](https://github.com/fastfetch-cli/fastfetch) | 67694107538abf20a46c00fb31f5a90463de5678 |
| [json-c](https://github.com/json-c/json-c)              | 0051f2dbe0cbcb537ccf257e9154b443488fecca |
| [libsndfile](https://github.com/libsndfile/libsndfile)  | c81375f070f3c6764969a738eacded64f53a076e |
| [mimalloc](https://github.com/microsoft/mimalloc)       | 4e50d6714d471b72b2285e25a3df6c92db944593 |
| [nanomsg](https://github.com/nanomsg/nanomsg)           | fc3f684a80151a3319446fc96083a9ff384ee4fe |
| [onion](https://github.com/davidmoreno/onion)           | de8ea938342b36c28024fd8393ebc27b8442a161 |
| [openssl](https://github.com/openssl/openssl)           | 6594baf6457c64f6fce3ec60cb2617f75d98d159 |
| [s2n-tls](https://github.com/aws/s2n-tls)               | b3ee9af0cf3c687892dd7ebd5624e5f48c6e394a |
| [zip](https://github.com/kuba--/zip)                    | 6f2116d77cdcbe544e4dbbf1e53895596ed14b89 |
| [zlog](https://github.com/HardySimpson/zlog)            | 7fe61ca6265516e9327a51fc394b2adb126c2ef3 |

#### C++
| Repository                                          | Commit                                   |
|-----------------------------------------------------|------------------------------------------|
| [benchmark](https://github.com/google/benchmark)    | bc946b919cac6f25a199a526da571638cfde109f |
| [fmt](https://github.com/fmtlib/fmt)                | f4b256c6676280dff9a9573c9b295414fd3e6861 |
| [glog](https://github.com/google/glog)              | 31429d85b8ffad6027dcf821242687f3c6c85df9 |
| [json](https://github.com/nlohmann/json)            | c883fb0f17cbdf75545bddcc551e21a924a31b05 |
| [leveldb](https://github.com/google/leveldb)        | 068d5ee1a3ac40dabd00d211d5013af44be55bea |
| [magic_enum](https://github.com/Neargye/magic_enum) | f34f967c4e70ec60d1c561fbd2fcabaeedce5957 |
| [MyTinySTL](https://github.com/Alinshans/MyTinySTL) | acc07e025f521a28773ff82c4d11e249911ddcb2 |
| [OpenCC](https://github.com/BYVoid/OpenCC)          | e5d6c5f1b78e28a5797e7ad3ede3513314e544b7 |
| [spdlog](https://github.com/gabime/spdlog)          | a2b4262090fd3f005c2315dcb5be2f0f1774a005 |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp)      | 76dc6715734295ff1866bfc32872ff2278258fc8 |

## Acknowledgements
[`stack-inspector`](./inspector/stack-inspector) approach was inspired by 
[EugeneDar/dynamotool](https://github.com/EugeneDar/dynamotool)
