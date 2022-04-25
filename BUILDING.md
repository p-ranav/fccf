# Building with CMake

## Build

This project doesn't require any special command-line flags to build to keep
things simple.

Here are the steps for building in release mode with a single-configuration
generator, like the Unix Makefiles one:

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-windows` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

To build tests, include the option -D fccf_DEVELOPER_MODE=ON

cmake -S . -B build -D fccf_DEVELOPER_MODE=ON
cmake --build build

## Install

This project doesn't require any special command-line flags to install to keep
things simple. As a prerequisite, the project has to be built with the above
commands already.

The below commands require at least CMake 3.15 to run, because that is the
version in which [Install a Project][1] was added.

Here is the command for installing the release mode artifacts with a
single-configuration generator, like the Unix Makefiles one:

```sh
cmake --install build
```

Here is the command for installing the release mode artifacts with a
multi-configuration generator, like the Visual Studio ones:

```sh
cmake --install build --config Release
```

### CMake package

This project exports a CMake package to be used with the [`find_package`][2]
command of CMake:

* Package name: `fccf`
* Cache variable: `FCCF_EXECUTABLE`

Example usage:

```cmake
find_package(fccf REQUIRED)
# Use the executable in some command
execute_process(
    COMMAND "${FCCF_EXECUTABLE}" ...
    ...
)
```

[1]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project
[2]: https://cmake.org/cmake/help/latest/command/find_package.html
