#include <fnmatch.h>
#include <searcher.hpp>
namespace fs = std::filesystem;

/* We want POSIX.1-2008 + XSI, i.e. SuSv4, features */
#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 700
#endif

/* If the C library can support 64-bit file sizes
   and offsets, using the standard names,
   these defines tell the C library to do so. */
#ifndef _LARGEFILE64_SOURCE
#  define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#endif

#include <errno.h>

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
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
#  define USE_FDS 15
#endif

namespace search
{
std::string_view::const_iterator needle_search(
    std::string_view needle,
    std::string_view::const_iterator haystack_begin,
    std::string_view::const_iterator haystack_end)
{
  if (haystack_begin != haystack_end) {
    return std::search(
        haystack_begin, haystack_end, needle.begin(), needle.end());
  } else {
    return haystack_end;
  }
}
  
void searcher::file_search(std::string_view filename,
			   std::string_view haystack)

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
    auto pos =
      sse2_strstr_v2(std::string_view(it, haystack_end - it), m_query);
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
  static const std::unordered_set<std::string_view> allowed_suffixes = {
      // C
      ".c",
      ".h"
      // C++
      ".cpp",
      ".cc",
      ".cxx",
      ".hh",
      ".hxx",
      ".hpp",
      // CUDA
      ".cu",
      ".cuh"
  };

  bool result = false;
  for (const auto& suffix : allowed_suffixes) {
    if (str.size() >= suffix.size()
        && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0)
    {
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

int handle_posix_directory_entry(const char* filepath,
                                 const struct stat* info,
                                 const int typeflag,
                                 struct FTW* pathinfo)
{
  static const bool skip_fnmatch =
      searcher::m_filter == std::string_view {"*.*"};

  if (typeflag == FTW_DNR) {
    // directory not readable
    return FTW_SKIP_SUBTREE;
  }

  if (typeflag == FTW_D || typeflag == FTW_DP) {
    // directory
    if (exclude_directory(filepath)) {
      return FTW_SKIP_SUBTREE;
    } else {
      return FTW_CONTINUE;
    }
  }

  if (typeflag == FTW_F) {
    if ((skip_fnmatch && is_whitelisted(filepath))
        || fnmatch(searcher::m_filter.data(), filepath, 0) == 0)
    {
      searcher::m_ts->push_task(
          [pathstring = std::string {filepath}]()
          { searcher::read_file_and_search(pathstring.data()); });
    }
  }

  return FTW_CONTINUE;
}

void searcher::directory_search(const char* path)
{
  /* Invalid directory path? */
  if (path == NULL || *path == '\0')
    return;

  nftw(
      path, handle_posix_directory_entry, USE_FDS, FTW_PHYS | FTW_ACTIONRETVAL);
  searcher::m_ts->wait_for_tasks();
}

}  // namespace search
