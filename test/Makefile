.PHONY: default clean

CC=`xcodebuild -find clang`
CXX=`xcodebuild -find clang++`

GOOGLE=../farmhash/src
PORTABLE=../portable

CPPFLAGS=-Os -DNDEBUG -Wall -arch i386 -arch x86_64 -I${GOOGLE} -I${PORTABLE} # -std=c++11

default: test
	arch -arch i386   ./test > test~i386.out
	arch -arch x86_64 ./test > test~x86_64.out
	cmp test~i386.out test~x86_64.out

test: test.cpp google.o portable.o
	${CXX} ${CPPFLAGS} $^ -o $@

test.o: test.cpp

google.o: ${GOOGLE}/farmhash.cc ${GOOGLE}/farmhash.h
	${CXX} ${CPPFLAGS} -Wno-unused-function -c ${GOOGLE}/farmhash.cc -o $@

portable.o: ${PORTABLE}/Endian.hpp ${PORTABLE}/FarmHash.cpp ${PORTABLE}/FarmHash.hpp ${PORTABLE}/FarmHash.o ${PORTABLE}/UInt128.hpp
	${CXX} ${CPPFLAGS} -c ${PORTABLE}/FarmHash.cpp -o $@

clean:
	rm -f test~i386.out test~x86_64.out test test.o google.o portable.o
