cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    fccf
    VERSION 0.6.0
    DESCRIPTION "fccf recursively searches a directory to find C/C++ source code matching a search string."
    HOMEPAGE_URL "https://github.com/p-ranav/fccf"
    LANGUAGES C CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

set(CMAKE_CXX_STANDARD 17)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# ---- Argparse -------------

set(CMAKE_MODULE_PATH "")
set(CMAKE_LIBRARY_ARCHITECTURE "")

include(FetchContent)

FetchContent_Declare(argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG master
)
FetchContent_MakeAvailable(argparse)

# ---- Fmt ----------------

set(FMT_HEADERS "")

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

# ---- nlohmann json ----------------

FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)
FetchContent_MakeAvailable(json)

# ---- LLVM ---------------

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Add path to LLVM modules
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${LLVM_CMAKE_DIR}"
  )

# import LLVM CMake functions
include(AddLLVM)

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

# LLVM link libraries
execute_process(COMMAND llvm-config --libs all
  OUTPUT_VARIABLE LLVM_LIBRARIES)
string(STRIP ${LLVM_LIBRARIES} LLVM_LIBRARIES)

# LLVM LD flags
execute_process(COMMAND llvm-config --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS)
string(STRIP ${LLVM_LDFLAGS} LLVM_LDFLAGS)

# LLVM CXX FLAGS
execute_process(COMMAND llvm-config --cxxflags
  OUTPUT_VARIABLE LLVM_CXXFLAGS)
string(STRIP ${LLVM_CXXFLAGS} LLVM_CXXFLAGS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

# ---- Threads ------------
if(NOT ANDROID)
  find_package(Threads REQUIRED)
endif()

## Append flags to enable exceptions and optimization
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fexceptions")

# ---- Declare library ----

add_library(
    fccf_lib OBJECT
    source/searcher.cpp
    source/sse2_strstr.cpp
    source/lexer.cpp
    source/utf8.cpp
)

target_include_directories(
    fccf_lib ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
    ${argparse_SOURCE_DIR}/include/argparse
)

target_compile_features(fccf_lib PUBLIC cxx_std_17)
target_compile_options (fccf_lib PRIVATE -fexceptions)
separate_arguments(LLVM_LDFLAGS UNIX_COMMAND "${LLVM_LDFLAGS}")
target_link_options(fccf_lib PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(fccf_lib PRIVATE fmt::fmt)
if(NOT ANDROID)
  target_link_libraries(fccf_lib Threads::Threads)
endif()

# ---- Declare executable ----

add_llvm_executable(fccf_exe source/main.cpp)

set_target_properties(
    fccf_exe PROPERTIES
    OUTPUT_NAME fccf
    EXPORT_NAME exe
)

target_compile_features(fccf_exe PRIVATE cxx_std_17)
target_compile_options(fccf_exe PRIVATE -fexceptions)
target_link_options(fccf_exe PRIVATE ${LLVM_LDFLAGS})
target_link_libraries(fccf_exe PRIVATE fccf_lib
  fmt::fmt
  ${LLVM_LIBRARIES}
  clang
  nlohmann_json::nlohmann_json
  )

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT fccf_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of fccf"
  )
endif()

include(cmake/dev-mode.cmake)
