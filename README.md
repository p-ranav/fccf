# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

<p align="center">
  <img src="https://user-images.githubusercontent.com/8450091/165386836-4370970c-78ae-4ae6-80d3-688ff253c972.gif"/> 
</p>

## Highlights

* Quickly identifies source files that contain a search string.
* For each candidate source file, builds an abstract syntax tree (AST).
* Visits the nodes in the AST, looking for function declarations, classes, enums, variables etc.
* Pretty-prints the identified snippet of source code to the terminal.
