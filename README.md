# fccf: Fast C/C++ Code Finder

`fccf` is a command-line tool that quickly searches through C/C++ source code in a directory based on a search string and prints relevant code snippets that match the query.

## Highlights

* Quickly identifies source files that contain a search string.
* For each candidate source file, builds an abstract syntax tree (AST).
* Visits the nodes in the AST, looking for function declarations, classes, enums, variables etc., that match the user's request.
* Pretty-prints the identified snippet of source code to the terminal.
* MIT License

## Searching the Linux kernel source tree

The following video shows `fccf` searching and finding snippets of code in [torvalds/linux](https://github.com/torvalds/linux/).

https://user-images.githubusercontent.com/8450091/165400381-9ba49a62-97fb-4f4a-890a-0dc9b20dfe75.mp4

## Searching the `fccf` source tree (Modern C++)

The following video shows `fccf` searching the `fccf` C++ source code. 

Note that search results here include:
1. Class declaration
2. Functions and function templates
3. Variable declarations, including lambda functions

https://user-images.githubusercontent.com/8450091/165402206-65d9ed43-b9dd-4528-92bd-0b4ce76b6468.mp4

## Searching expressions

Use the `--include-expressions` option to find expressions that match the query

<img width="895" alt="image" src="https://user-images.githubusercontent.com/8450091/165763755-af0f37e2-da98-4991-9806-a90199c7a8cd.png">

<img width="744" alt="image" src="https://user-images.githubusercontent.com/8450091/165763662-80256fcf-676a-4ec7-ad86-a92cb769dc82.png">

## Build Instructions

Build `fccf` using CMake. For more details, see [BUILDING.md](https://github.com/p-ranav/fccf/blob/master/BUILDING.md).

NOTE: `fccf` requires `libclang` and `LLVM` installed.

```bash
# Install libclang and LLVM
# sudo apt install libclang-dev llvm

git clone https://github.com/p-ranav/fccf
cd fccf

# Build
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build

# Install
sudo cmake --install build
```

## `fccf` Arguments

```console
~/dev/fccf$ fccf --help
1 argument(s) expected. 0 provided.
Usage: search [options] query path

Positional arguments:
query
path

Optional arguments:
-h --help                               shows help message and exits [default: false]
-v --version                            prints version information and exits [default: false]
-h --help                               Shows help message and exits [default: false]
-E --exact-match                        Only consider exact matches [default: false]
-f --filter                             Only evaluate files that match filter pattern [default: "*.*"]
-j                                      Number of threads [default: 5]
--enum                                  Search for enum declaration [default: false]
--struct                                Search for struct declaration [default: false]
--union                                 Search for union declaration [default: false]
--member-function                       Search for class member function declaration [default: false]
--function                              Search for function declaration [default: false]
--function-template                     Search for function template declaration [default: false]
-F                                      Search for any function or function template or class member function [default: false]
--class                                 Search for class declaration [default: false]
--class-template                        Search for class template declaration [default: false]
--class-constructor                     Search for class constructor declaration [default: false]
--isl --ignore-single-line-results      Ignore forward declarations, member function declarations, etc. [default: false]
--typedef                               Search for typedef declaration [default: false]
--verbose                               Request verbose output [default: false]
-C                                      Search for any class or class template or struct [default: false]
-I --include-dir                        Additional include directories [default: {}]
-l --language                           Language option used by clang [default: "c++"]
--std                                   C++ standard to be used by clang [default: "c++17"]
```
