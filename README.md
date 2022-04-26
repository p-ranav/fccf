# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

## Highlights

* Quickly identifies source files that contain a search string.
* Uses [clang-c](https://clang.llvm.org/doxygen/group__CINDEX.html) to build an AST of each source file.
* Analyzes nodes ('CXCursor' abstraction) in the AST for each translation unit.
* Finds nodes that match the user query, e.g., class declaration, function template declaration, enum declaration etc.
* Pretty-prints the identified snippet of source code to the terminal

<p align="center">
  <img height="600" src="images/demo.png"/>  
</p>
