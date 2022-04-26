# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

* Quickly identifies source files that contain a search string.
* For each candidate source file, builds an abstract syntax tree (AST).
* Visits the nodes in the AST, looking for function declarations, classes, enums, variables etc., that match the user's request.
* Pretty-prints the identified snippet of source code to the terminal.

The following demo video shows `fccf` searching and finding snippets of code in [torvalds/linux](https://github.com/torvalds/linux/).

https://user-images.githubusercontent.com/8450091/165400381-9ba49a62-97fb-4f4a-890a-0dc9b20dfe75.mp4

