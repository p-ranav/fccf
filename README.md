# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

* Quickly identifies source files that contain a search string.
* For each candidate source file, builds an abstract syntax tree (AST).
* Visits the nodes in the AST, looking for function declarations, classes, enums, variables etc., that match the user's request.
* Pretty-prints the identified snippet of source code to the terminal.

The following demo video shows `fccf` searching and finding snippets of code in [torvalds/linux](https://github.com/torvalds/linux/).

https://user-images.githubusercontent.com/8450091/165400066-76c88459-f95b-4256-8106-c2e2ba1e3153.mp4
