#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#define FMT_HEADER_ONLY 1
#include <fmt/color.h>
#include <fmt/core.h>
#include <thread_pool.hpp>

namespace search
{
using custom_printer_callback =
    std::function<void(std::string_view filename,
                       bool is_stdout,
                       unsigned start_line,
                       unsigned end_line,
                       std::string_view code_snippet)>;
struct searcher
{
  static inline std::unique_ptr<thread_pool> m_ts;
  static inline std::string_view m_query;
  static inline std::vector<std::string> m_filters;
  static inline std::vector<std::string> m_excludes;
  static inline bool m_no_ignore_dirs;
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
  static inline bool m_search_for_class_destructor;
  static inline bool m_search_for_typedef;
  static inline bool m_search_for_using_declaration;
  static inline bool m_search_for_namespace_alias;
  static inline bool m_ignore_single_line_results;
  static inline bool m_search_expressions;
  static inline bool m_search_for_variable_declaration;
  static inline bool m_search_for_parameter_declaration;
  static inline bool m_search_for_static_cast;
  static inline bool m_search_for_dynamic_cast;
  static inline bool m_search_for_reinterpret_cast;
  static inline bool m_search_for_const_cast;
  static inline bool m_search_for_throw_expression;
  static inline bool m_search_for_for_statement;
  static inline custom_printer_callback m_custom_printer;

  static void file_search(std::string_view filename, std::string_view haystack);
  static void read_file_and_search(const char* path);
  static void directory_search(const char* path);
};

}  // namespace search
