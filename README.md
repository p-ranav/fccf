# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

https://user-images.githubusercontent.com/8450091/165315616-c8918375-63cb-4f45-8313-46a38f3e20f6.mp4

## Highlights

* Quickly identifies source files that contain a search string.
* For each candidate source file, builds an abstract syntax tree (AST).
* Visits the nodes in the AST, looking for function declarations, classes, enums, variables etc.
* Pretty-prints the identified snippet of source code to the terminal.
