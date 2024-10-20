#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <argparse.hpp>
#include <nlohmann/json.hpp>
#include "searcher.hpp"
#include <unistd.h>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
  auto is_stdout = isatty(STDOUT_FILENO) == 1;
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);
  argparse::ArgumentParser program("fccf", "0.6.0");
  program.add_argument("query");
  program.add_argument("path").remaining();

  // Generic Program Information
  program.add_argument("-h", "--help")
      .help("Shows help message and exits")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-E", "--exact-match")
      .help("Only consider exact matches")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--json")
      .help("Print results in JSON format")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-f", "--filter")
      .help("Only evaluate files that match any of these glob patterns")
      .default_value<std::vector<std::string>>({"*.*"})
      .append();

  program.add_argument("-e", "--exclude")
      .help("Only evaluate files that DO NOT match any of these glob patterns")
      .default_value<std::vector<std::string>>({})
      .append();

  program.add_argument("--no-ignore-dirs")
      .help(
          "Do not ignore files under an integrated list of blocklisted "
          "directories (VCS, IDE, common build directories, etc.)")
      .default_value(false)
      .implicit_value(true);

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
      .help(
          "Search for any function or function template or class member "
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

  program.add_argument("--class-destructor")
      .help("Search for class destructor declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-C")
      .help("Search for any class or class template or struct")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--for-statement")
      .help("Search for `for` statement")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--namespace-alias")
      .help("Search for namespace alias")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--parameter-declaration")
      .help("Search for function or method parameter")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--typedef")
      .help("Search for typedef declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--using-declaration")
      .help(
          "Search for using declarations, using directives, and type alias "
          "declarations")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--variable-declaration")
      .help("Search for variable declaration")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--verbose")
      .help("Request verbose output")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--ie", "--include-expressions")
      .help(
          "Search for expressions that refer to some value or "
          "member, e.g., function, variable, or enumerator.")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--static-cast")
      .help("Search for static_cast")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--dynamic-cast")
      .help("Search for dynamic_cast")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--reinterpret-cast")
      .help("Search for reinterpret_cast")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--const-cast")
      .help("Search for const_cast")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-c")
      .help(
          "Search for any static_cast, dynamic_cast, reinterpret_cast, or"
          "const_cast expression")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--throw-expression")
      .help("Search for throw expression")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--isl", "--ignore-single-line-results")
      .help("Ignore forward declarations, member function declarations, etc.")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-I", "--include-dir")
      .default_value<std::vector<std::string>>({})
      .append()
      .help("Additional include directories");

  program.add_argument("-l", "--language")
      .default_value<std::string>(std::string {"c++"})
      .help("Language option used by clang");

  program.add_argument("--std")
      .default_value<std::string>(std::string {"c++17"})
      .help("C++ standard to be used by clang");

  program.add_argument("--nc", "--no-color")
      .help("Stops fccf from coloring the output")
      .default_value(false)
      .implicit_value(true);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    if (program.get<bool>("--help")) {
      std::cout << program << std::endl;
      return 0;
    } else {
      std::cerr << err.what() << std::endl;
      std::cerr << program;
      std::exit(1);
    }
  }

  std::vector<std::string> paths;
  try {
    paths = program.get<std::vector<std::string>>("path");
  } catch (const std::logic_error& e) {
    // No path provided
    paths = {"."};
  }
  auto query = program.get<std::string>("query");
  auto exact_match = program.get<bool>("--exact-match");

  auto filters = program.get<std::vector<std::string>>("-f");
  auto excludes = program.get<std::vector<std::string>>("-e");
  auto no_ignore_dirs = program.get<bool>("--no-ignore-dirs");

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
  auto search_for_class_destructor = program.get<bool>("--class-destructor");
  auto search_for_any_class_or_struct = program.get<bool>("-C");
  auto search_for_typedef = program.get<bool>("--typedef");
  auto search_for_using_declaration = program.get<bool>("--using-declaration");
  auto search_for_namespace_alias = program.get<bool>("--namespace-alias");
  auto search_expressions = program.get<bool>("--include-expressions");
  auto search_for_variable_declaration =
      program.get<bool>("--variable-declaration");
  auto search_for_parameter_declaration =
      program.get<bool>("--parameter-declaration");

  auto search_for_static_cast = program.get<bool>("--static-cast");
  auto search_for_dynamic_cast = program.get<bool>("--dynamic-cast");
  auto search_for_reinterpret_cast = program.get<bool>("--reinterpret-cast");
  auto search_for_const_cast = program.get<bool>("--const-cast");
  auto search_for_any_cast = program.get<bool>("-c");

  auto search_for_throw_expression = program.get<bool>("--throw-expression");

  auto search_for_for_statement = program.get<bool>("--for-statement");

  auto verbose = program.get<bool>("--verbose");
  auto include_dirs = program.get<std::vector<std::string>>("--include-dir");
  auto language_option = program.get<std::string>("--language");
  auto cpp_std = program.get<std::string>("--std");
  auto ignore_single_line_results =
      program.get<bool>("--ignore-single-line-results");

  auto no_color = program.get<bool>("--no-color");
  auto is_json = program.get<bool>("--json");

  if (no_color) {
    is_stdout = false;
  }

  auto no_filter =
      !(search_for_enum || search_for_struct || search_for_union
        || search_for_member_function || search_for_function
        || search_for_function_template || search_for_any_function
        || search_for_class || search_for_class_template
        || search_for_class_constructor || search_for_class_destructor
        || search_for_any_class_or_struct || search_for_typedef
        || search_for_using_declaration || search_for_namespace_alias
        || search_for_variable_declaration || search_for_parameter_declaration
        || search_for_static_cast || search_for_dynamic_cast
        || search_for_reinterpret_cast || search_for_const_cast
        || search_for_any_cast || search_for_throw_expression
        || search_for_for_statement);

  auto ends_with = [](std::string_view str, std::string_view suffix) -> bool
  {
    return str.size() >= suffix.size()
        && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
  };

  std::vector<std::string> include_directory_list;  // {"-I."};

  for (auto& id : include_dirs) {
    include_directory_list.push_back("-I" + id);
  }

  // Configure a searcher
  search::searcher searcher;
  searcher.m_query = std::move(query);
  searcher.m_filters = std::move(filters);
  searcher.m_excludes = std::move(excludes);
  searcher.m_no_ignore_dirs = no_ignore_dirs;
  searcher.m_is_stdout = is_stdout;
  searcher.m_verbose = verbose;
  searcher.m_exact_match = exact_match;
  searcher.m_search_for_enum = no_filter || search_for_enum;
  searcher.m_search_for_struct =
      no_filter || search_for_any_class_or_struct || search_for_struct;
  searcher.m_search_for_union = no_filter || search_for_union;
  searcher.m_search_for_member_function =
      no_filter || search_for_any_function || search_for_member_function;
  searcher.m_search_for_function =
      no_filter || search_for_any_function || search_for_function;
  searcher.m_search_for_function_template =
      no_filter || search_for_any_function || search_for_function_template;
  searcher.m_search_for_class =
      no_filter || search_for_any_class_or_struct || search_for_class;
  searcher.m_search_for_class_template =
      no_filter || search_for_any_class_or_struct || search_for_class_template;
  searcher.m_search_for_class_constructor =
      no_filter || search_for_class_constructor;
  searcher.m_search_for_class_destructor =
      no_filter || search_for_class_destructor;
  searcher.m_search_for_typedef = no_filter || search_for_typedef;
  searcher.m_search_for_using_declaration =
      no_filter || search_for_using_declaration;
  searcher.m_search_for_namespace_alias =
      no_filter || search_for_namespace_alias;
  searcher.m_search_for_variable_declaration =
      no_filter || search_for_variable_declaration;
  searcher.m_search_for_parameter_declaration =
      no_filter || search_for_parameter_declaration;
  searcher.m_search_expressions = search_expressions;
  searcher.m_search_for_static_cast =
      no_filter || search_for_any_cast || search_for_static_cast;
  searcher.m_search_for_dynamic_cast =
      no_filter || search_for_any_cast || search_for_dynamic_cast;
  searcher.m_search_for_reinterpret_cast =
      no_filter || search_for_any_cast || search_for_reinterpret_cast;
  searcher.m_search_for_const_cast =
      no_filter || search_for_any_cast || search_for_const_cast;

  searcher.m_search_for_throw_expression =
      no_filter || search_for_throw_expression;

  searcher.m_search_for_for_statement = no_filter || search_for_for_statement;
  searcher.m_ignore_single_line_results = ignore_single_line_results;
  searcher.m_ts = std::make_unique<thread_pool>(num_threads);

  nlohmann::json json_array = nlohmann::json::array();
  if (is_json) {
    searcher.m_custom_printer = [&json_array](std::string_view filename,
                                              bool is_stdout,
                                              unsigned start_line,
                                              unsigned end_line,
                                              std::string_view code_snippet)
    {
      nlohmann::json obj;
      obj["filename"] = filename;
      obj["snippet"] = code_snippet;
      obj["start_line"] = start_line;
      obj["end_line"] = end_line;
      json_array.push_back(obj);
    };
  }

  for (const auto& path : paths) {
    // Update clang options
    auto parent_path = path == "." ? "." : fs::path(path).parent_path();
    auto parent_path_string = parent_path.c_str();

    // Iterate over the `std::filesystem::directory_entry` elements using `auto`
    for (auto const& dir_entry : fs::recursive_directory_iterator(parent_path))
    {
      auto& path = dir_entry.path();
      if (fs::is_directory(path)) {
        // If directory name is include
        std::string_view directory_name = path.filename().c_str();
        if (ends_with(directory_name, "include")) {
          include_directory_list.push_back("-I" + std::string {path});
        }
      }
    }

    std::vector<const char*> clang_options;
    clang_options.push_back("-x");
    clang_options.push_back(language_option.c_str());

    auto language_standard = "-std=" + cpp_std;
    if (language_option == "c++") {
      clang_options.push_back(language_standard.c_str());
    }

    for (auto& include_directory : include_directory_list) {
      clang_options.push_back(include_directory.c_str());
    }

    searcher.m_clang_options = clang_options;

    // Run the search

    if (fs::is_regular_file(fs::path(path))) {
      searcher.read_file_and_search((const char*)path.c_str());
    } else if (fs::is_directory(fs::path(path))) {
      searcher.directory_search((const char*)path.c_str());
    } else {
      fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold,
                 "\nError: '{}' is not a valid file or directory\n",
                 path);
      std::exit(1);
    }
  }

  if (is_json) {
    fmt::print("{}", json_array.dump());
  }
  fmt::print("\n");
  return 0;
}
