CXXFLAGS=-std=c++11 -Wall -Wextra -Wzero-as-null-pointer-constant -Wuseless-cast -Wconversion -Winline -pedantic -Wfatal-errors -g -I./include
CXXOPTIMIZATIONFLAGS=
COPTIMIZATIONFLAGS=
DYNAMIC_SYMS=-Wl,--dynamic-list-cpp-typeinfo

PREFIX=/usr
BIN_PATH=${PREFIX}/bin
LIB_PATH=${PREFIX}/lib/pjac
H_PATH=${PREFIX}/include/pjac

LIBDL ?= -ldl

.SUFFIXES: .cpp .h .o

.PHONY: all remake clean


############################################################
# BASICS
all: build/bin/pjac

remake: clean all


build/bin/pjac: src/main.cpp
	$(CXX) $(CXXFLAGS) $(CXXOPTIMIZATIONFLAGS) -o $@ $^


############################################################
# CLEANING
clean:
	rm -f ./build/bin/*
	rm -f ./build/*.o


############################################################
# PLATFORM OBJECT FILES
build/platform/exception.o: src/types/exception.cpp
	${CXX} -std=c++11 -fPIC -c -I./include -o ./build/platform/exception.o src/types/exception.cpp


############################################################
# TESTING
build/test/World.o: sample/asm/external/World.cpp
	${CXX} ${CXXFLAGS} ${CXXOPTIMIZATIONFLAGS} -c -fPIC -o build/test/World.o ./sample/asm/external/World.cpp

build/test/math.so: build/test/math.o build/platform/registerset.o build/platform/exception.o build/platform/type.o build/platform/pointer.o
	${CXX} ${CXXFLAGS} ${CXXOPTIMIZATIONFLAGS} -fPIC -shared -o build/test/math.so build/test/math.o ./build/platform/registerset.o ./build/platform/exception.o build/platform/type.o build/platform/pointer.o
