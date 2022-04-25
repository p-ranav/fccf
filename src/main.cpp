#include <argparse.hpp>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <searcher.hpp>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>
namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  const auto is_stdout = isatty(STDOUT_FILENO) == 1;
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);
  argparse::ArgumentParser program("search", "0.2.0\n");
  program.add_argument("query");
  program.add_argument("path");

  // Generic Program Information
  program.add_argument("-h", "--help")
      .help("Shows help message and exits")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--exact-match")
      .help("Only consider exact matches")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-f", "--filter")
      .help("Only evaluate files that match filter pattern")
      .default_value(std::string{"*.*"});

  program.add_argument("-j")
      .help("Number of threads")
      .scan<'d', int>()
      .default_value(5);

  program.add_argument("--enum")
      .help("Search for enum declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--struct")
      .help("Search for struct declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--union")
      .help("Search for union declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--member-function")
      .help("Search for class member function declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--function")
      .help("Search for function declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--function-template")
      .help("Search for function template declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-F")
      .help("Search for any function or function template or class member "
            "function")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--class")
      .help("Search for class declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--class-template")
      .help("Search for class template declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--class-constructor")
      .help("Search for class constructor declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--typedef")
      .help("Search for typedef declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--verbose")
      .help("Request verbose output")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-C")
      .help("Search for any class or class template or struct")
      .default_value(false)
      .implicit_value(true);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  auto query = program.get<std::string>("query");
  auto path = program.get<std::string>("path");
  auto exact_match = program.get<bool>("--exact-match");
  auto filter = program.get<std::string>("-f");
  auto num_threads = program.get<int>("-j");
  auto search_for_enum = program.get<bool>("--enum");
  auto search_for_struct = program.get<bool>("--struct");
  auto search_for_union = program.get<bool>("--union");
  auto search_for_member_function = program.get<bool>("--member-function");
  auto search_for_function = program.get<bool>("--function");
  auto search_for_function_template = program.get<bool>("--function-template");
  auto search_for_any_function = program.get<bool>("-F");
  auto search_for_class = program.get<bool>("--class");
  auto search_for_class_template = program.get<bool>("--class-template");
  auto search_for_class_constructor = program.get<bool>("--class-constructor");
  auto search_for_any_class_or_struct = program.get<bool>("-C");
  auto search_for_typedef = program.get<bool>("--typedef");
  auto verbose = program.get<bool>("--verbose");

  auto no_filter = !(search_for_enum || search_for_struct || search_for_union ||
                     search_for_member_function || search_for_function ||
                     search_for_function_template || search_for_any_function ||
                     search_for_class || search_for_class_template ||
                     search_for_class_constructor);

  auto ends_with = [](std::string_view str, std::string_view suffix) -> bool {
    return str.size() >= suffix.size() &&
           0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
  };

  std::vector<std::string> include_directory_list; // {"-I."};

  // Iterate over the `std::filesystem::directory_entry` elements using `auto`
  for (auto const &dir_entry : fs::recursive_directory_iterator(path)) {
    auto &path = dir_entry.path();
    if (fs::is_directory(path)) {
      // If directory name is include
      std::string_view directory_name = path.filename().c_str();
      if (ends_with(directory_name, "include")) {
        include_directory_list.push_back("-I" + std::string{path});
      }
    //   else {
    //     for (const auto& include_entry : fs::directory_iterator(path)) {
    //       if (fs::is_regular_file(include_entry.path())) {
    //         auto possible_include_path = include_entry.path().c_str();
    //         if (ends_with(possible_include_path, ".h") || 
    //             ends_with(possible_include_path, ".hpp")) {
    //             include_directory_list.push_back("-I" + std::string{possible_include_path});
    //             break;
    //         }
    //       }
    //     }
    //   }
    }
  }

  std::vector<const char *> clang_options;
  clang_options.push_back("-x");
  clang_options.push_back("c++");
  clang_options.push_back("-std=c++17");
  for (auto &include_directory : include_directory_list) {
    clang_options.push_back(include_directory.c_str());
  }

  // Configure a searcher
  search::searcher searcher;
  searcher.m_query = query;
  searcher.m_filter = filter;
  searcher.m_is_stdout = is_stdout;
  searcher.m_verbose = verbose;
  searcher.m_clang_options = clang_options;
  searcher.m_exact_match = exact_match;
  searcher.m_search_for_enum = no_filter || search_for_enum;
  searcher.m_search_for_struct = no_filter || search_for_any_class_or_struct || search_for_struct;
  searcher.m_search_for_union = no_filter || search_for_union;
  searcher.m_search_for_member_function =
      no_filter || search_for_any_function || search_for_member_function;
  searcher.m_search_for_function =
      no_filter || search_for_any_function || search_for_function;
  searcher.m_search_for_function_template =
      no_filter || search_for_any_function || search_for_function_template;
  searcher.m_search_for_class = no_filter || search_for_any_class_or_struct || search_for_class;
  searcher.m_search_for_class_template = no_filter || search_for_any_class_or_struct || search_for_class_template;
  searcher.m_search_for_class_constructor =
      no_filter || search_for_class_constructor;
  searcher.m_search_for_typedef = no_filter || search_for_typedef;
  searcher.m_ts = std::make_unique<thread_pool>(num_threads);
  searcher.directory_search(path.c_str());
  fmt::print("\n");
  return 0;
}
