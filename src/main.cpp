#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <clang-c/Index.h>  // This is libclang.
#include <string_view>
#include <argparse.hpp>
#include <searcher.hpp>
#include <filesystem>
namespace fs = std::filesystem;

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

int main(int argc, char* argv[])
{
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(NULL);
  argparse::ArgumentParser program("search", "0.2.0\n");
  program.add_argument("query");
  program.add_argument("path").remaining();

  // Generic Program Information
  program.add_argument("-h", "--help")
      .help("Shows help message and exits")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("-f", "--filter")
      .help("Only evaluate files that match filter pattern")
      .default_value(std::string {"*.[c|h](pp?)"});

  program.add_argument("-j")
      .help("Number of threads")
      .scan<'d', int>()
      .default_value(5);

  program.parse_args(argc, argv);

  auto query = program.get<std::string>("query");
  auto filter = program.get<std::string>("-f");
  auto num_threads = program.get<int>("-j");

  // Configure a searcher
  search::searcher searcher;
  searcher.m_query = query;
  searcher.m_filter = filter;
  searcher.m_ts = std::make_unique<thread_pool>(num_threads);  
  searcher.directory_search(".");
  
  static const char* path = argv[1];

  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    path, nullptr, 0,
    nullptr, 0,
    CXTranslationUnit_None);
  if (unit == nullptr)
  {
    std::cerr << "Unable to parse translation unit. Quitting.\n";
    std::exit(-1);
  }  

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(
    cursor,
    [](CXCursor c, CXCursor parent, CXClientData client_data)
    {
      // Namespace check before recursing
      if (c.kind == CXCursor_Namespace) {	
	auto namespace_name =
	  (const char*)clang_getCursorDisplayName(c).data;	
	if (strcmp(namespace_name, "search") == 0) {
	  return CXChildVisit_Recurse;	  
	}
      }
      
      // CXX Class Member function
      // Prints class::member_function_name with line number
      if (c.kind == CXCursor_CXXMethod) {	
	auto source_range = clang_getCursorExtent(c);
	auto start_location = clang_getRangeStart(source_range);
	auto end_location = clang_getRangeEnd(source_range);

	CXFile file;
	unsigned start_line, start_column, start_offset;
	clang_getExpansionLocation(start_location, &file, &start_line, &start_column,
				   &start_offset);

	unsigned end_line, end_column, end_offset;
	clang_getExpansionLocation(end_location, &file, &end_line, &end_column,
				   &end_offset);		

	if (strstr((const char*)clang_getCursorDisplayName(c).data, "directory_search")) {
	  /*std::cout << (const char*)clang_getCursorDisplayName(clang_getCursorSemanticParent(c)).data
		    << "::";	
	  std::cout << (const char*)clang_getCursorDisplayName(c).data
		    << ":" << start_line << ":" << end_line << "\n";
	  */
	  // TODO: If this is the function you need
	  // then return CXChildVisit_Break;
	  
	  auto contents = get_file_contents(path);
	  if (!contents.empty()) {
	    std::cout << contents.substr(source_range.begin_int_data - 2, source_range.end_int_data - source_range.begin_int_data) << "\n";
	  }
	}
	
      }
      return CXChildVisit_Continue;
    },
    nullptr);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  return 0;
}
