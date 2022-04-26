# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

## Highlights

* Quickly identifies source files that contain a search string.
* Uses [clang-c](https://clang.llvm.org/doxygen/group__CINDEX.html) to build an AST of each source file.
* Analyzes nodes in the AST for each translation unit.
* Finds nodes that match the user query, e.g., class declaration, function template declaration, enum declaration etc.
* Pretty-prints the identified snippet of source code to the terminal.

https://user-images.githubusercontent.com/8450091/165304800-84b3c1be-26a0-47c4-8cb4-61a4dd3252ad.mp4

