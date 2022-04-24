#pragma once
#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <streambuf>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>

#include <immintrin.h>
#include <sse2_strstr.hpp>
#include <thread_pool.hpp>

namespace search {
struct searcher {
  static inline std::unique_ptr<thread_pool> m_ts;
  static inline std::string_view m_query;
  static inline std::string_view m_filter;
  static inline std::vector<const char *> m_clang_options;

  static void file_search(std::string_view filename, std::string_view haystack);
  static void read_file_and_search(const char *path);
  static void directory_search(const char *path);
};

} // namespace search
