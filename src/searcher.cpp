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

void searcher::file_search(std::string_view filename, std::string_view haystack)

{
  auto out = fmt::memory_buffer();

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
    // TODO: Print with --verbose option
    // fmt::format_to(std::back_inserter(out), "? {}\n", path);

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index, path, m_clang_options.data(), m_clang_options.size(), nullptr, 0,
        CXTranslationUnit_None);
    if (unit == nullptr) {
      fmt::print("Error: Unable to parse translation unit {}. Quitting.\n",
                 path);
      std::exit(-1);
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    struct client_args {
      std::string_view filename;
      std::string_view haystack;
      fmt::memory_buffer *out;
    };
    client_args args = {filename, haystack, &out};

    if (clang_visitChildren(
            cursor,
            [](CXCursor c, CXCursor parent, CXClientData client_data) {
              client_args *args = (client_args *)client_data;
              auto filename = args->filename;
              auto haystack = args->haystack;
              auto &out = *(args->out);

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

                    auto haystack_size = haystack.size();
                    auto pos = source_range.begin_int_data - 2;
                    auto count =
                        source_range.end_int_data - source_range.begin_int_data;

                    if (pos < haystack_size) {
                      // Filename
                      if (m_is_stdout) {
                        fmt::format_to(std::back_inserter(out),
                                       "\033[1;31m{}\033[0m ", filename);
                      } else {
                        fmt::format_to(std::back_inserter(out), "{} ",
                                       filename);
                      }

                      // Line number (start, end)
                      if (m_is_stdout) {
                        fmt::format_to(std::back_inserter(out),
                                       "\033[1;36m(Line: {} to {})\033[0m\n",
                                       start_line, end_line);
                      } else {
                        fmt::format_to(std::back_inserter(out),
                                       "(Line: {} to {})\n", start_line,
                                       end_line);
                      }

                      fmt::format_to(std::back_inserter(out), "{}\n\n",
                                     haystack.substr(pos, count));
                    }
                  }
                }
              }
              return CXChildVisit_Recurse;
            },
            (void *)(&args))) {
      fmt::print("Error: Visit children failed for {}\n)", path);
    }

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

    fmt::print("{}", fmt::to_string(out));
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
      ".c", ".h",
      // C++
      ".cpp", ".cc", ".cxx", ".hh", ".hxx", ".hpp",
      // CUDA
      ".cu", ".cuh"};

  bool result = false;
  for (const auto &suffix : allowed_suffixes) {
    if (std::equal(suffix.rbegin(), suffix.rend(), str.rbegin())) {
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
    bool consider_file = false;
    if (skip_fnmatch && is_whitelisted(filepath)) {
      consider_file = true;
    } else if (!skip_fnmatch &&
               fnmatch(searcher::m_filter.data(), filepath, 0) == 0) {
      consider_file = true;
    }
    if (consider_file) {
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
