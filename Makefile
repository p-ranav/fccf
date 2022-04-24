CXX := g++
LLVMCOMPONENTS := all
RTTIFLAG := -fno-rtti
LLVMCONFIG := llvm-config
CXXFLAGS := -I. -O3 $(shell $(LLVMCONFIG) --cxxflags) $(RTTIFLAG) -std=c++17
LLVMLDFLAGS := $(shell $(LLVMCONFIG) --ldflags --libs $(LLVMCOMPONENTS))
SOURCES = main.cpp sse2_strstr.cpp
OBJECTS = main.o
EXES = $(OBJECTS:.o=)
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

all: $(OBJECTS) $(EXES)

%: %.o
	$(CXX) -o $@ $< $(CLANGLIBS) $(LLVMLDFLAGS)
