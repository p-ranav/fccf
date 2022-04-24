#ifndef LEXER_H
#define LEXER_H
#include <algorithm>
#include <string_view>
#include <token.hpp>
#include <unordered_set>
#include <vector>

#define FMT_HEADER_ONLY 1
#include <fmt/color.h>
#include <fmt/core.h>

class lexer {
  std::string_view m_input;
  fmt::memory_buffer *m_out;
  std::size_t m_index{0};

  char previous() const;
  char current() const;
  char next() const;
  void move_forward(std::size_t n = 1);
  bool is_line_comment();
  bool is_block_comment();
  bool is_start_of_identifier();
  bool is_start_of_string();
  bool is_start_of_number();
  void process_line_comment();
  void process_block_comment();
  void process_identifier();
  void process_string();
  std::size_t get_number_of_characters(std::string_view str);

public:
  void tokenize_and_pretty_print(std::string_view source,
                                 fmt::memory_buffer *out);
};

#endif