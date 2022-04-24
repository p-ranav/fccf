CXX := g++
LLVMCOMPONENTS := all
RTTIFLAG := -fno-rtti
LLVMCONFIG := llvm-config
CXXFLAGS := -Isrc -O3 $(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG) -std=c++17 -fexceptions
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))
SOURCES = src/main.cpp src/sse2_strstr.cpp src/searcher.cpp src/lexer.cpp src/utf8.cpp
CLANGLIBS = \
	-lclang\
	-lclangTooling\
	-lclangFrontendTool\
	-lclangFrontend\
	-lclangDriver\
	-lclangSerialization\
	-lclangCodeGen\
	-lclangParse\
	-lclangSema\
	-lclangStaticAnalyzerFrontend\
	-lclangStaticAnalyzerCheckers\
	-lclangStaticAnalyzerCore\
	-lclangAnalysis\
	-lclangARCMigrate\
	-lclangEdit\
	-lclangAST\
	-lclangLex\
	-lclangBasic\
	$(shell $(LLVMCONFIG) --libs)

LINK_LIBS = $(CLANGLIBS) $(LLVMLDFLAGS) -lpthread

all:
	$(CXX) $(CXXFLAGS) -o main $(SOURCES) $(LINK_LIBS)
