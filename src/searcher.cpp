#include <fnmatch.h>
#include <searcher.hpp>
namespace fs = std::filesystem;

/* We want POSIX.1-2008 + XSI, i.e. SuSv4, features */
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

/* If the C library can support 64-bit file sizes
   and offsets, using the standard names,
   these defines tell the C library to do so. */
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <errno.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <ftw.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* POSIX.1 says each process has at least 20 file descriptors.
 * Three of those belong to the standard streams.
 * Here, we use a conservative estimate of 15 available;
 * assuming we use at most two for other uses in this program,
 * we should never run into any problems.
 * Most trees are shallower than that, so it is efficient.
 * Deeper trees are traversed fine, just a bit slower.
 * (Linux allows typically hundreds to thousands of open files,
 *  so you'll probably never see any issues even if you used
 *  a much higher value, say a couple of hundred, but
 *  15 is a safe, reasonable value.)
 */
#ifndef USE_FDS
#define USE_FDS 15
#endif

#include <clang-c/Index.h> // This is libclang.

#include <utility>

namespace search {
std::string_view::const_iterator
needle_search(std::string_view needle,
              std::string_view::const_iterator haystack_begin,
              std::string_view::const_iterator haystack_end) {
  if (haystack_begin != haystack_end) {
    return std::search(haystack_begin, haystack_end, needle.begin(),
                       needle.end());
  } else {
    return haystack_end;
  }
}

auto find_needle_position(std::string_view str, std::string_view query) {
  auto it = needle_search(query, str.begin(), str.end());

  return it != str.end() ? std::size_t(it - str.begin())
                         : std::string_view::npos;
}

void print_colored(std::string_view str, std::string_view query) {
  auto pos = find_needle_position(str, query);
  if (pos == std::string_view::npos) {
    std::cout << str << "\n";
    return;
  } else {
    std::cout << str.substr(0, pos) << "\033[1;31m"
              << str.substr(pos, query.size()) << "\033[0m"
              << str.substr(pos + query.size()) << "\n";
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
    const char *path = filename.data();

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index, path, m_clang_options.data(), m_clang_options.size(), nullptr, 0,
        CXTranslationUnit_None);
    if (unit == nullptr) {
      std::cerr << "Unable to parse translation unit. Quitting.\n";
      std::exit(-1);
    }

    std::pair<std::string_view, std::string_view> filename_and_contents = {
        filename, haystack};

    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    if (clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data) {
              auto filename_and_contents =
                  (std::pair<std::string_view, std::string_view> *)client_data;
              auto filename = filename_and_contents->first;
              auto haystack = filename_and_contents->second;

              // CXX Class Member function
              // Prints class::member_function_name with line number
              if (c.kind == CXCursor_CXXMethod ||
                  c.kind == CXCursor_FunctionDecl ||
                  c.kind == CXCursor_FunctionTemplate) {
                auto source_range = clang_getCursorExtent(c);
                auto start_location = clang_getRangeStart(source_range);
                auto end_location = clang_getRangeEnd(source_range);

                CXFile file;
                unsigned start_line, start_column, start_offset;
                clang_getExpansionLocation(start_location, &file, &start_line,
                                           &start_column, &start_offset);

                unsigned end_line, end_column, end_offset;
                clang_getExpansionLocation(end_location, &file, &end_line,
                                           &end_column, &end_offset);

                if (end_line >= start_line) {

                  std::string_view member_function_name =
                      (const char *)clang_getCursorDisplayName(c).data;
                  std::string_view query = searcher::m_query.data();

                  if (member_function_name.find(query) !=
                      std::string_view::npos) {
                    /*std::cout << (const
                      char*)clang_getCursorDisplayName(clang_getCursorSemanticParent(c)).data
                      << "::";
                      std::cout << (const
                      char*)clang_getCursorDisplayName(c).data
                      << ":" << start_line << ":" << end_line << "\n";
                    */

                    auto haystack_size = haystack.size();
                    auto pos = source_range.begin_int_data - 2;
                    auto count =
                        source_range.end_int_data - source_range.begin_int_data;

                    if (pos < haystack_size) {
                      if (c.kind == CXCursor_CXXMethod) {
                        std::cout << "\n\033[1;36m"
                                  << "[MEMBER FUNCTION]"
                                  << "\033[0m ";
                      } else if (c.kind == CXCursor_FunctionDecl) {
                        std::cout << "\n\033[1;36m"
                                  << "[FUNCTION]"
                                  << "\033[0m ";
                      } else if (c.kind == CXCursor_FunctionTemplate) {
                        std::cout << "\n\033[1;36m"
                                  << "[FUNCTION TEMPLATE]"
                                  << "\033[0m ";
                      }
                      std::cout << "\033[1;32m" << filename << "\033[0m (";
                      std::cout << "\033[1;37m"
                                << "Line: " << start_line << " to " << end_line
                                << ")\033[0m\n";
                      print_colored(haystack.substr(pos, count), query);
                    }
                  }
                }
              }
              return CXChildVisit_Recurse;
            },
            (void *)(&filename_and_contents))) {
      std::cerr << "Visit children failed for " << path << "\n";
    }

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
  }
}

std::string get_file_contents(const char *filename) {
  std::FILE *fp = std::fopen(filename, "rb");
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

void searcher::read_file_and_search(const char *path) {
  const std::string haystack = get_file_contents(path);
  file_search(path, haystack);
}

bool is_whitelisted(const std::string_view &str) {
  static const std::unordered_set<std::string_view> allowed_suffixes = {
      // C
      ".c",
      ".h"
      // C++
      ".cpp",
      ".cc", ".cxx", ".hh", ".hxx", ".hpp",
      // CUDA
      ".cu", ".cuh"};

  bool result = false;
  for (const auto &suffix : allowed_suffixes) {
    if (str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0) {
      result = true;
      break;
    }
  }
  return result;
}

bool exclude_directory(const char *path) {
  static const std::unordered_set<const char *> ignored_dirs = {
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

  for (const auto &ignored_dir : ignored_dirs) {
    // if path contains ignored dir, ignore it
    if (strstr(path, ignored_dir) != nullptr) {
      return true;
    }
  }

  return false;
}

int handle_posix_directory_entry(const char *filepath, const struct stat *info,
                                 const int typeflag, struct FTW *pathinfo) {
  static const bool skip_fnmatch =
      searcher::m_filter == std::string_view{"*.*"};

  if (typeflag == FTW_F) {
    if ((skip_fnmatch && is_whitelisted(filepath)) ||
        fnmatch(searcher::m_filter.data(), filepath, 0) == 0) {
      searcher::m_ts->push_task([pathstring = std::string{filepath}]() {
        searcher::read_file_and_search(pathstring.data());
      });
    }
  }

  return FTW_CONTINUE;
}

void searcher::directory_search(const char *path) {
  /* Invalid directory path? */
  if (path == NULL || *path == '\0')
    return;

  nftw(path, handle_posix_directory_entry, USE_FDS,
       FTW_PHYS | FTW_ACTIONRETVAL);
  searcher::m_ts->wait_for_tasks();
}

} // namespace search
