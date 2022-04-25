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

#define FMT_HEADER_ONLY 1
#include <fmt/color.h>
#include <fmt/core.h>
#include <immintrin.h>
#include <sse2_strstr.hpp>
#include <thread_pool.hpp>

namespace search
{
struct searcher
{
  static inline std::unique_ptr<thread_pool> m_ts;
  static inline std::string_view m_query;
  static inline std::string_view m_filter;
  static inline bool m_verbose;
  static inline bool m_is_stdout;
  static inline std::vector<const char*> m_clang_options;
  static inline bool m_exact_match;
  static inline bool m_search_for_enum;
  static inline bool m_search_for_struct;
  static inline bool m_search_for_union;
  static inline bool m_search_for_member_function;
  static inline bool m_search_for_function;
  static inline bool m_search_for_function_template;
  static inline bool m_search_for_class;
  static inline bool m_search_for_class_template;
  static inline bool m_search_for_class_constructor;
  static inline bool m_search_for_typedef;
  static inline bool m_ignore_single_line_results;

  static void file_search(std::string_view filename, std::string_view haystack);
  static void read_file_and_search(const char* path);
  static void directory_search(const char* path);
};

}  // namespace search
