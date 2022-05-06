# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(fccfTests LANGUAGES CXX)

find_package(Clang REQUIRED CONFIG)

# ---- Tests ----

add_executable(fccf_test source/fccf_test.cpp)
target_link_libraries(fccf_test PRIVATE fccf_lib libclang)
target_compile_features(fccf_test PRIVATE cxx_std_17)

add_test(NAME fccf_test COMMAND fccf_test)

# ---- End-of-file commands ----

add_folders(Test)
