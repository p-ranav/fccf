# fccf: Fast C/C++ Code Finder

fccf recursively searches a directory to find C/C++ source code based on a search string.

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

