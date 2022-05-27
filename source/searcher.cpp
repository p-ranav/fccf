#include <fnmatch.h>
#include <lexer.hpp>
#include <searcher.hpp>
namespace fs = std::filesystem;

#include <utility>

#include <clang-c/Index.h>  // This is libclang.

namespace
{
void print_code_snippet(std::string_view filename,
                    bool is_stdout,
                    unsigned start_line,
                    unsigned end_line,
                    std::string_view code_snippet)
{
  auto out = fmt::memory_buffer();
  if (is_stdout) {
    fmt::format_to(
        std::back_inserter(out), "\n\033[1;90m// {}\033[0m ", filename);
  } else {
    fmt::format_to(std::back_inserter(out), "\n// {} ", filename);
  }

  if (is_stdout) {
    fmt::format_to(std::back_inserter(out),
                   "\033[1;90m(Line: {} to {})\033[0m\n",
                   start_line,
                   end_line);
  } else {
    fmt::format_to(
        std::back_inserter(out), "(Line: {} to {})\n", start_line, end_line);
  }
  lexer lex;
  lex.tokenize_and_pretty_print(code_snippet, &out, is_stdout);
  fmt::format_to(std::back_inserter(out), "\n");
  fmt::print("{}", fmt::to_string(out));
}
}  // namespace
namespace search
{
auto needle_search(std::string_view needle,
                   std::string_view::const_iterator haystack_begin,
                   std::string_view::const_iterator haystack_end)
    -> std::string_view::const_iterator
{
  if (haystack_begin != haystack_end) {
    return std::search(
        haystack_begin, haystack_end, needle.begin(), needle.end());
  } else {
    return haystack_end;
  }
}

void searcher::file_search(std::string_view filename, std::string_view haystack)

{
  // Start from the beginning
  const auto haystack_begin = haystack.cbegin();
  const auto haystack_end = haystack.cend();

  auto it = haystack_begin;
  bool first_search = true;
  bool printed_file_name = false;
  std::size_t current_line_number = 1;
  auto no_file_name = filename.empty();

#if defined(__SSE2__)
  std::string_view view(it, haystack_end - it);
  if (view.empty()) {
    it = haystack_end;
  } else {
    auto pos = sse2_strstr_v2(std::string_view(it, haystack_end - it), m_query);
    if (pos != std::string::npos) {
      it += pos;
    } else {
      it = haystack_end;
    }
  }
#else
  it = needle_search(m_query, it, haystack_end);
#endif

  if (it != haystack_end) {
    // analyze file
    const char* path = filename.data();
    if (searcher::m_verbose) {
      fmt::print("Checking {}\n", path);
    }

    // Update a copy of the clang options
    // Include
    auto clang_options = m_clang_options;
    auto parent_path = fs::path(filename).parent_path();
    auto parent_path_str = "-I" + parent_path.string();
    auto grandparent_path = parent_path.parent_path();
    auto grandparent_path_str = "-I" + grandparent_path.string();
    clang_options.push_back(parent_path_str.c_str());
    clang_options.push_back(grandparent_path_str.c_str());
    clang_options.push_back("-I/usr/include");
    clang_options.push_back("-I/usr/local/include");

    if (m_verbose) {
      fmt::print("Clang options:\n");
      for (auto& option : clang_options) {
        fmt::print("{} ", option);
      }
      fmt::print("\n");
    }

    CXIndex index;

    if (searcher::m_verbose) {
      index = clang_createIndex(0, 1);
    } else {
      index = clang_createIndex(0, 0);
    }
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index,
        path,
        clang_options.data(),
        clang_options.size(),
        nullptr,
        0,
        CXTranslationUnit_KeepGoing
            | CXTranslationUnit_IgnoreNonErrorsFromIncludedFiles);
    // CXTranslationUnit_None);
    if (unit == nullptr) {
      fmt::print("Error: Unable to parse translation unit {}. Quitting.\n",
                 path);
      std::exit(-1);
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    struct client_args
    {
      std::string_view filename;
      std::string_view haystack;
      custom_printer_callback printer;
    };
    client_args args = {filename, haystack, m_custom_printer};

    if (clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data)
            {
              client_args* args = (client_args*)client_data;
              auto filename = args->filename;
              auto haystack = args->haystack;
              auto printer = args->printer;

              /*
                CXCursor_CXXStaticCastExpr
                C++'s static_cast<> expression.

                CXCursor_CXXDynamicCastExpr
                C++'s dynamic_cast<> expression.

                CXCursor_CXXReinterpretCastExpr
                C++'s reinterpret_cast<> expression.

                CXCursor_CXXConstCastExpr
                C++'s const_cast<> expression.
              */

              if ((searcher::m_search_expressions
                   && (c.kind == CXCursor_DeclRefExpr
                       || c.kind == CXCursor_MemberRefExpr
                       || c.kind == CXCursor_MemberRef
                       || c.kind == CXCursor_FieldDecl))
                  || (searcher::m_search_for_enum
                      && c.kind == CXCursor_EnumDecl)
                  || (searcher::m_search_for_struct
                      && c.kind == CXCursor_StructDecl)
                  || (searcher::m_search_for_union
                      && c.kind == CXCursor_UnionDecl)
                  || (searcher::m_search_for_member_function
                      && c.kind == CXCursor_CXXMethod)
                  || (searcher::m_search_for_function
                      && c.kind == CXCursor_FunctionDecl)
                  || (searcher::m_search_for_function_template
                      && c.kind == CXCursor_FunctionTemplate)
                  || (searcher::m_search_for_class
                      && c.kind == CXCursor_ClassDecl)
                  || (searcher::m_search_for_class_template
                      && c.kind == CXCursor_ClassTemplate)
                  || (searcher::m_search_for_class_constructor
                      && c.kind == CXCursor_Constructor)
                  || (searcher::m_search_for_class_destructor
                      && c.kind == CXCursor_Destructor)
                  || (searcher::m_search_for_typedef
                      && c.kind == CXCursor_TypedefDecl)
                  || (searcher::m_search_for_using_declaration
                      && (c.kind == CXCursor_UsingDirective
                          || c.kind == CXCursor_UsingDeclaration
                          || c.kind == CXCursor_TypeAliasDecl))
                  || (searcher::m_search_for_namespace_alias
                      && c.kind == CXCursor_NamespaceAlias)
                  || (searcher::m_search_for_variable_declaration
                      && c.kind == CXCursor_VarDecl)
                  || (searcher::m_search_for_parameter_declaration
                      && c.kind == CXCursor_ParmDecl)
                  || (searcher::m_search_for_static_cast
                      && c.kind == CXCursor_CXXStaticCastExpr)
                  || (searcher::m_search_for_dynamic_cast
                      && c.kind == CXCursor_CXXDynamicCastExpr)
                  || (searcher::m_search_for_reinterpret_cast
                      && c.kind == CXCursor_CXXReinterpretCastExpr)
                  || (searcher::m_search_for_const_cast
                      && c.kind == CXCursor_CXXConstCastExpr)
                  || (searcher::m_search_for_throw_expression
                      && c.kind == CXCursor_CXXThrowExpr)
                  || (searcher::m_search_for_for_statement
                      && (c.kind == CXCursor_ForStmt
                          || c.kind == CXCursor_CXXForRangeStmt)))
              {
                // fmt::print("Found something in {}\n", filename);

                auto source_range = clang_getCursorExtent(c);
                auto start_location = clang_getRangeStart(source_range);
                auto end_location = clang_getRangeEnd(source_range);

                CXFile file;
                unsigned start_line, start_column, start_offset;
                clang_getExpansionLocation(start_location,
                                           &file,
                                           &start_line,
                                           &start_column,
                                           &start_offset);

                unsigned end_line, end_column, end_offset;
                clang_getExpansionLocation(
                    end_location, &file, &end_line, &end_column, &end_offset);

                if ((!searcher::m_ignore_single_line_results
                     && end_line >= start_line)
                    || (m_ignore_single_line_results && end_line > start_line))
                {
                  std::string_view name =
                      (const char*)clang_getCursorSpelling(c).data;
                  std::string_view query = searcher::m_query.data();

                  if (query.empty()
                      || (
                          // The query check for these is done
                          // a little later down the road
                          // (once a code snippet is available
                          // to check against)
                          searcher::m_search_for_throw_expression
                          || searcher::m_search_for_typedef
                          || searcher::m_search_for_static_cast
                          || searcher::m_search_for_dynamic_cast
                          || searcher::m_search_for_reinterpret_cast
                          || searcher::m_search_for_const_cast
                          || searcher::m_search_for_for_statement)
                      || (searcher::m_exact_match && name == query
                          && c.kind != CXCursor_DeclRefExpr
                          && c.kind != CXCursor_MemberRefExpr
                          && c.kind != CXCursor_MemberRef
                          && c.kind != CXCursor_FieldDecl)
                      || (!searcher::m_exact_match
                          && name.find(query) != std::string_view::npos))
                  {
                    auto haystack_size = haystack.size();
                    auto pos = source_range.begin_int_data - 2;
                    auto count =
                        source_range.end_int_data - source_range.begin_int_data;

                    // fmt::print("{} - Pos: {}, Count: {}, Haystack size:
                    // {}\n", filename, pos, count, haystack_size);

                    if ((searcher::m_search_expressions
                         && (c.kind == CXCursor_DeclRefExpr
                             || c.kind == CXCursor_MemberRefExpr
                             || c.kind == CXCursor_MemberRef
                             || c.kind == CXCursor_FieldDecl)))
                    {
                      // Update pos and count so that the entire line of code is
                      // printed instead of just the reference (e.g., variable
                      // name)
                      auto newline_before = haystack.rfind('\n', pos);
                      while (haystack[newline_before + 1] == ' '
                             || haystack[newline_before + 1] == '\t')
                      {
                        newline_before += 1;
                      }
                      auto newline_after = haystack.find('\n', pos);
                      pos = newline_before + 1;
                      count = newline_after - newline_before - 1;
                    }

                    if (pos < haystack_size) {
                      auto code_snippet = haystack.substr(pos, count);

                      // Handles throw expression, static_cast,
                      // dynamic_cast, const_cast, reinterpret_cast
                      // for_statement, and ranged_for_statement
                      //
                      // if the `query` is part of the code snippet,
                      // then show result, else, skip it
                      if (searcher::m_search_for_throw_expression
                          || searcher::m_search_for_typedef
                          || searcher::m_search_for_static_cast
                          || searcher::m_search_for_dynamic_cast
                          || searcher::m_search_for_reinterpret_cast
                          || searcher::m_search_for_const_cast
                          || searcher::m_search_for_for_statement)
                      {
                        if (code_snippet.find(query) == std::string_view::npos)
                        {
                          // skip result
                          return CXChildVisit_Continue;
                        }
                      }
                      if (printer) {
                        printer(filename,
                                m_is_stdout,
                                start_line,
                                end_line,
                                code_snippet);
                      } else {
                        print_code_snippet(filename,
                                       m_is_stdout,
                                       start_line,
                                       end_line,
                                       code_snippet);
                      }

                      // Line number (start, end)
                    }
                  }
                }
              }
              return CXChildVisit_Recurse;
            },
            (void*)(&args)))
    {
      fmt::print("Error: Visit children failed for {}\n)", path);
    }

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
  }
}

std::string get_file_contents(const char* filename)
{
  std::FILE* fp = std::fopen(filename, "rb");
  if (fp) {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    const auto size = std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return (contents);
  }
  return "";
}

void searcher::read_file_and_search(const char* path)
{
  const std::string haystack = get_file_contents(path);
  file_search(path, haystack);
}

bool is_whitelisted(const std::string_view& str)
{
  static const std::unordered_set<std::string_view> allowed_suffixes = {// C
                                                                        ".c",
                                                                        ".h",
                                                                        // C++
                                                                        ".cpp",
                                                                        ".cc",
                                                                        ".cxx",
                                                                        ".hh",
                                                                        ".hxx",
                                                                        ".hpp",
                                                                        // CUDA
                                                                        ".cu",
                                                                        ".cuh"};

  bool result = false;
  for (const auto& suffix : allowed_suffixes) {
    if (std::equal(suffix.rbegin(), suffix.rend(), str.rbegin())) {
      result = true;
      break;
    }
  }
  return result;
}

bool exclude_directory(const char* path)
{
  static const std::unordered_set<const char*> ignored_dirs = {
      ".git/",         ".github/",       "build/",
      "node_modules/", ".vscode/",       ".DS_Store/",
      "debugPublic/",  "DebugPublic/",   "debug/",
      "Debug/",        "Release/",       "release/",
      "Releases/",     "releases/",      "cmake-build-debug/",
      "__pycache__/",  "Binaries/",      "Doc/",
      "doc/",          "Documentation/", "docs/",
      "Docs/",         "bin/",           "Bin/",
      "patches/",      "tar-install/",   "CMakeFiles/",
      "install/",      "snap/",          "LICENSES/",
      "img/",          "images/",        "imgs/"};

  for (const auto& ignored_dir : ignored_dirs) {
    // if path contains ignored dir, ignore it
    if (strstr(path, ignored_dir) != nullptr) {
      return true;
    }
  }

  return false;
}

void searcher::directory_search(const char* search_path)
{
  static const bool skip_fnmatch =
      searcher::m_filter == std::string_view {"*.*"};

  for (auto const& dir_entry : fs::recursive_directory_iterator(search_path)) {
    auto& path = dir_entry.path();
    const char* path_string = (const char*)path.c_str();
    if (fs::is_regular_file(path)) {
      bool consider_file = false;
      if (skip_fnmatch && is_whitelisted(path_string)) {
        consider_file = true;
      } else if (!skip_fnmatch
                 && fnmatch(searcher::m_filter.data(), path_string, 0) == 0)
      {
        consider_file = true;
      }
      if (consider_file) {
        searcher::m_ts->push_task(
            [pathstring = std::string {path_string}]()
            { searcher::read_file_and_search(pathstring.data()); });
      }
    }
  }
  searcher::m_ts->wait_for_tasks();
}

}  // namespace search
