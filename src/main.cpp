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
  program.add_argument("path").remaining();

  // Generic Program Information
  program.add_argument("-h", "--help")
      .help("Shows help message and exits")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-f", "--filter")
      .help("Only evaluate files that match filter pattern")
      .default_value(std::string{"*.*"});

  program.add_argument("-j")
      .help("Number of threads")
      .scan<'d', int>()
      .default_value(5);

  program.parse_args(argc, argv);

  auto query = program.get<std::string>("query");
  auto filter = program.get<std::string>("-f");
  auto num_threads = program.get<int>("-j");

  auto ends_with = [](std::string_view str, std::string_view suffix) -> bool {
    return str.size() >= suffix.size() &&
           0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
  };

  std::vector<std::string> include_directory_list;

  // Iterate over the `std::filesystem::directory_entry` elements using `auto`
  for (auto const &dir_entry : fs::recursive_directory_iterator(".")) {
    auto &path = dir_entry.path();
    if (fs::is_directory(path)) {
      // If directory name is include
      std::string_view directory_name = path.filename().c_str();
      if (ends_with(directory_name, "include")) {
        include_directory_list.push_back("-I" + std::string{path});
      }
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
  searcher.m_clang_options = clang_options;
  searcher.m_ts = std::make_unique<thread_pool>(num_threads);
  searcher.directory_search(".");
  return 0;
}
