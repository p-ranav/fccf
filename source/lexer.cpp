#include <clocale>
#include <iostream>

#include <lexer.hpp>
#include <token.hpp>
#include <utf8.h>

char lexer::previous() const { return m_input[m_index - 1]; }

char lexer::current() const { return m_input[m_index]; }

char lexer::next() const { return m_input[m_index + 1]; }

void lexer::move_forward(std::size_t n) { m_index += n; }

bool lexer::is_line_comment() {
  if (m_index + 1 < m_input.size()) {
    if (current() == '/' && next() == '/') {
      return true;
    }
  }
  return false;
}

bool lexer::is_block_comment() {
  if (m_index + 1 < m_input.size()) {
    if (current() == '/' && next() == '*') {
      return true;
    }
  }
  return false;
}

bool lexer::is_start_of_identifier() {
  char c = current();
  if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_') ||
      ((unsigned char)c >= 0x80)) {
    return true;
  } else {
    return false;
  }
}

bool lexer::is_start_of_string() {
  if (current() == '"' || current() == '\'') {
    return true;
  }
  return false;
}

bool lexer::is_start_of_number() {
  if (isdigit(current())) {
    return true;
  }
  return false;
}

void lexer::process_line_comment() {
  while (m_index < m_input.size()) {
    fmt::format_to(std::back_inserter(*m_out), "\033[0;90m{}\033[0m",
                   current());
    move_forward();
    if (current() == '\n' && previous() != '\\') {
      break;
    }
  }
}

void lexer::process_block_comment() {
  std::size_t level{0};
  while (m_index < m_input.size()) {
    if (current() == '/' && next() == '*') {
      ++level;
      fmt::format_to(std::back_inserter(*m_out), "\033[0;90m/*\033[0m");
      move_forward(2);
    } else if (current() == '*' && next() == '/') {
      --level;
      fmt::format_to(std::back_inserter(*m_out), "\033[0;90m*/\033[0m");
      move_forward(2);
      if (level == 0) {
        break;
      }
    } else if (previous() != '\\' && current() == '\n') {
      fmt::format_to(std::back_inserter(*m_out), "\033[0;90m{}\033[0m",
                     current());
      move_forward();
    } else {
      fmt::format_to(std::back_inserter(*m_out), "\033[0;90m{}\033[0m",
                     current());
      move_forward();
    }
  }
}

bool lexer::process_identifier(bool maybe_class_or_struct) {
  auto start = m_index;
  while (m_index < m_input.size()) {
    char c = current();
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || (c == '_') || ((unsigned char)c >= 0x80)) {
      move_forward();
    } else {
      break;
    }
  }
  auto end = m_index;
  std::string_view identifier(m_input.data() + start, end - start);

  static const std::unordered_set<std::string_view> keywords{"alignas",
                                                             "alignof",
                                                             "and",
                                                             "and_eq",
                                                             "asm",
                                                             "atomic_cancel",
                                                             "atomic_commit",
                                                             "atomic_noexcept",
                                                             "bitand",
                                                             "bitor",
                                                             "break",
                                                             "case",
                                                             "catch",
                                                             "class",
                                                             "compl",
                                                             "concept",
                                                             "consteval",
                                                             "constexpr",
                                                             "constinit",
                                                             "const",
                                                             "const_cast",
                                                             "continue",
                                                             "co_await",
                                                             "co_return",
                                                             "co_yield",
                                                             "decltype",
                                                             "default",
                                                             "delete",
                                                             "do",
                                                             "dynamic_cast",
                                                             "else",
                                                             "explicit",
                                                             "export",
                                                             "extern",
                                                             "for",
                                                             "friend",
                                                             "goto",
                                                             "if",
                                                             "inline",
                                                             "mutable",
                                                             "namespace",
                                                             "new",
                                                             "noexcept",
                                                             "not",
                                                             "not_eq",
                                                             "nullptr",
                                                             "operator",
                                                             "or",
                                                             "or_eq",
                                                             "private",
                                                             "protected",
                                                             "public",
                                                             "reflexpr",
                                                             "register",
                                                             "reinterpret_cast",
                                                             "requires",
                                                             "return",
                                                             "sizeof",
                                                             "static",
                                                             "static_assert",
                                                             "static_cast",
                                                             "struct",
                                                             "switch",
                                                             "synchronized",
                                                             "template",
                                                             "this",
                                                             "thread_local",
                                                             "throw",
                                                             "try",
                                                             "typedef",
                                                             "typeid",
                                                             "typename",
                                                             "union",
                                                             "using",
                                                             "virtual",
                                                             "void",
                                                             "volatile",
                                                             "while",
                                                             "xor",
                                                             "xor_eq"};

  static const std::unordered_set<std::string_view> types{"auto",
                                                          "bool",
                                                          "char",
                                                          "char8_t",
                                                          "char16_t",
                                                          "char32_t",
                                                          "double",
                                                          "enum",
                                                          "false",
                                                          "float",
                                                          "int",
                                                          "int8_t",
                                                          "int16_t",
                                                          "int32_t",
                                                          "int64_t",
                                                          "uint8_t",
                                                          "uint16_t",
                                                          "uint32_t",
                                                          "uint64_t",
                                                          "long",
                                                          "short",
                                                          "signed",
                                                          "size_t"
                                                          "true",
                                                          "unsigned",
                                                          "wchar_t"};

  // Check if identifier in keywords set
  if (keywords.find(identifier) != keywords.end()) {
    // Keyword
    // Color it Purple
    fmt::format_to(std::back_inserter(*m_out), "\033[1;95m{}\033[0m",
                   identifier);
  } else if (types.find(identifier) != types.end()) {
    // Type
    // Color it Blue
    fmt::format_to(std::back_inserter(*m_out), "\033[1;94m{}\033[0m",
                   identifier);
  } else {
    if (current() == ':') {
      // Label
      // COlor it green
      fmt::format_to(std::back_inserter(*m_out), "\033[1;92m{}\033[0m",
                     identifier);
    } else if (current() == '.') {
      // Member
      // Color it cyan
      fmt::format_to(std::back_inserter(*m_out), "\033[1;96m{}\033[0m",
                     identifier);
    } else if (current() == '(') {
      // Function
      // Color it yellow
      fmt::format_to(std::back_inserter(*m_out), "\033[1;93m{}\033[0m",
                     identifier);
    } else if (maybe_class_or_struct) {
      // Class or Struct name 
      // Color it Blue
      fmt::format_to(std::back_inserter(*m_out), "\033[1;94m{}\033[0m",
                    identifier);
    } else {
      // Identifier
      fmt::format_to(std::back_inserter(*m_out), "{}", identifier);
    }
  }

  if (identifier == "class" || identifier == "struct") {
    return true;
  }
  else {
    return false;
  }
}

void lexer::process_string() {
  auto start = m_index;
  move_forward();
  bool escape = false;
  while (m_index < m_input.size()) {
    if (current() == '\\' && !escape) {
      escape = true;
      move_forward();
      continue;
    }

    if (!escape && (current() == '"' || current() == '\'')) {
      break;
    } else if (escape) {
      escape = false;
    }

    move_forward();
  }
  move_forward();

  auto end = m_index;
  std::string_view literal(m_input.data() + start, end - start);
  fmt::format_to(std::back_inserter(*m_out), "\033[1;91m{}\033[0m", literal);
}

std::size_t lexer::get_number_of_characters(std::string_view str) {
  int count = 0;
  for (std::size_t i = 0; i < str.size();) {
    char c = str[i];
    auto len = u8_seqlen(&c);
    i += len;
    count += 1;
  }
  return count;
}

void lexer::tokenize_and_pretty_print(std::string_view input,
                                      fmt::memory_buffer *out) {

  bool class_or_struct_keyword_encountered = false;

  if (!input.empty()) {
    m_input = input;
    m_out = out;
    while (m_index < m_input.size()) {
      auto c = input[m_index];

      if (c == '/') {
        if (is_line_comment()) {
          process_line_comment();
          continue;
        } else if (is_block_comment()) {
          process_block_comment();
          continue;
        } else {
          fmt::format_to(std::back_inserter(*out), "{}", c);
          move_forward();
          continue;
        }
      } else if (is_start_of_identifier()) {
        class_or_struct_keyword_encountered = process_identifier(class_or_struct_keyword_encountered);
      } else if (is_start_of_string()) {
        process_string();
      } else {
        fmt::format_to(std::back_inserter(*out), "{}", c);
        move_forward();
      }
    }
  }
}