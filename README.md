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

## Search for any of `--flag` that matches
  
Provide an empty query to match any `--flag`, e.g., any enum declaration.
  
<img width="697" alt="image" src="https://user-images.githubusercontent.com/8450091/165854416-bc3e0cd1-042b-4f8c-b653-dd43ecae1e3a.png">

... or any class constructor

<img width="694" alt="image" src="https://user-images.githubusercontent.com/8450091/165858122-ecfaf103-8e84-418f-8aaa-8f0fc1d087ea.png">

## Searching a `for` statement 

Use `--for-statement` to search `for` statements. `fccf` will try to find `for` statements (including C++ ranged `for`) that contain the query string.

<img width="694" alt="image" src="https://user-images.githubusercontent.com/8450091/165875203-0c30623f-b935-4dcd-b929-1053e0348d12.png">

## Searching expressions

Use the `--include-expressions` option to find expressions that match the query.

The following example shows `fccf` find calls to `isdigit()`.

<img width="700" alt="image" src="https://user-images.githubusercontent.com/8450091/165769520-8c640847-280e-4d7d-8c6a-ce2cc108cdaf.png">

The following example shows `fccf` finding references to the `clang_options` variable.

<img width="575" alt="image" src="https://user-images.githubusercontent.com/8450091/165769421-6d6141ff-0f00-45f6-9396-92a56abbd308.png">

## Searching for `using` declarations

Use the `--using-declaration` option to find `using` declarations, `using` directives, and type alias declarations.

<img width="750" alt="image" src="https://user-images.githubusercontent.com/8450091/165815095-cce0d7c1-3568-4a1b-b5f1-427e163fa81b.png">

## Searching for `namespace` aliases

Use the `--namespace-alias` option to find `namespace` aliases.

<img width="400" alt="image" src="https://user-images.githubusercontent.com/8450091/165815204-bfe3f900-82cd-4411-acc5-0f1b63ec926e.png">

## Searching for `throw` expressions

Use `--throw-expression` with a query to search for specific `throw` expressions that contain the query string.

<img width="740" alt="image" src="https://user-images.githubusercontent.com/8450091/165873571-652d43d1-03be-4569-8329-2e481795f330.png">

As presented earlier, an empty query here will attempt to match any `throw` expression in the code base:

<img width="800" alt="image" src="https://user-images.githubusercontent.com/8450091/165873839-70730714-a7bc-46d8-8ea7-6be5438e374b.png">

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

## `fccf` Usage

```console
foo@bar:~$ fccf --help
Usage: fccf [options] query path

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
--class-destructor                      Search for class destructor declaration [default: false]
-C                                      Search for any class or class template or struct [default: false]
--for-statement                         Search for `for` statement [default: false]
--namespace-alias                       Search for namespace alias [default: false]
--parameter-declaration                 Search for function or method parameter [default: false]
--typedef                               Search for typedef declaration [default: false]
--using-declaration                     Search for using declarations, using directives, and type alias declarations [default: false]
--variable-declaration                  Search for variable declaration [default: false]
--verbose                               Request verbose output [default: false]
--ie --include-expressions              Search for expressions that refer to some value or member, e.g., function, variable, or enumerator. [default: false]
--static-cast                           Search for static_cast [default: false]
--dynamic-cast                          Search for dynamic_cast [default: false]
--reinterpret-cast                      Search for reinterpret_cast [default: false]
--const-cast                            Search for const_cast [default: false]
-c                                      Search for any static_cast, dynamic_cast, reinterpret_cast, orconst_cast expression [default: false]
--throw-expression                      Search for throw expression [default: false]
--isl --ignore-single-line-results      Ignore forward declarations, member function declarations, etc. [default: false]
-I --include-dir                        Additional include directories [default: {}]
-l --language                           Language option used by clang [default: "c++"]
--std                                   C++ standard to be used by clang [default: "c++17"]
```

## How it works

1. `fccf` does a recursive directory search for a needle in a haystack - like `grep` or `ripgrep` - It uses an `SSE2` `strstr` SIMD algorithm (modified Rabin-Karp SIMD search; see [here](http://0x80.pl/articles/simd-strfind.html)) if possible to quickly find, in multiple threads, a subset of the source files in the directory that contain a needle.
2. For each candidate source file, it uses `libclang` to parse the translation unit (build an abstract syntax tree).
3. Then it visits each child node in the AST, looking for specific node types, e.g., `CXCursor_FunctionDecl` for function declarations.
4. Once the relevant nodes are identified, if the node's "spelling" (`libclang` name for the node) matches the search query, then the source range of the AST node is identified - source range is the start and end index of the snippet of code in the buffer
5. Then, it pretty-prints this snippet of code. I have a simple lexer that tokenizes this code and prints colored output.

### Note on `include_directories`

For all this to work, fccf first identifies candidate directories that contain header files, e.g., paths that end with `include/`. It then adds these paths to the clang options (before parsing the translation unit) as `-Ifoo -Ibar/baz` etc. Additionally, for each translation unit, the parent and grandparent paths are also added to the include directories for that unit in order to increase the likelihood of successful parsing.

Additional include directories can also be provided to `fccf` using the `-I` or `--include-dir` option. Using verbose output (`--verbose`), errors in the libclang parsing can be identified and fixes can be attempted (e.g., adding the right include directories so that `libclang` is happy).

To run `fccf` on the `fccf` source code without any libclang errors, I had to explicitly provide the include path from LLVM-12 like so:

```console
foo@bar:~$ fccf --verbose 'lexer' . --include-dir /usr/lib/llvm-12/include/
Checking ./source/lexer.cpp
Checking ./source/lexer.hpp
Checking ./source/searcher.cpp

// ./source/lexer.hpp (Line: 14 to 40)
class lexer
{
  std::string_view m_input;
  fmt::memory_buffer* m_out;
  std::size_t m_index {0};
  bool m_is_stdout {true};

  char previous() const;
  char current() const;
  char next() const;
  void move_forward(std::size_t n = 1);
  bool is_line_comment();
  bool is_block_comment();
  bool is_start_of_identifier();
  bool is_start_of_string();
  bool is_start_of_number();
  void process_line_comment();
  void process_block_comment();
  bool process_identifier(bool maybe_class_or_struct = false);
  void process_string();
  std::size_t get_number_of_characters(std::string_view str);

public:
  void tokenize_and_pretty_print(std::string_view source,
                                 fmt::memory_buffer* out,
                                 bool is_stdout = true);
}
```
