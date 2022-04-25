#ifndef TOKEN_H
#define TOKEN_H
#include <cstddef>

enum class token_type {
  newline,

  // Single-character tokens.
  left_paren,
  right_paren,
  left_brace,
  right_brace,
  left_bracket,
  right_bracket,
  dot,
  comma,
  colon,
  semicolon,
  plus,
  minus,
  asterisk,
  slash,
  percent,
  ampersand,
  pipe,
  caret,
  tilde,
  question,

  // One or two character tokens.
  ampersand_ampersand,
  pipe_pipe,
  bang,
  bang_equal,
  equal,
  equal_equal,
  greater,
  greater_equal,
  less,
  less_equal,
  plus_equal,
  minus_equal,
  asterisk_equal,
  slash_equal,
  percent_equal,
  colon_equal,
  colon_colon,

  // Literals.
  identifier,
  string,
  number,

  // Keywords.
  keyword_if,
  keyword_else,
  keyword_while,
  keyword_for,
  keyword_break,
  keyword_continue,
  keyword_return,
  keyword_true,
  keyword_false,

  // End of file.
  eof
};

struct token {
  token_type type;
  std::size_t start_index;
  std::size_t end_index;
  std::size_t line;
  std::size_t start_col;
  std::size_t end_col;

  token(token_type type, std::size_t start_index, std::size_t end_index,
        std::size_t line, std::size_t start_col, std::size_t end_col)
      : type(type), start_index(start_index), end_index(end_index), line(line),
        start_col(start_col), end_col(end_col) {}
};

#endif