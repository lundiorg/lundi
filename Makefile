all: test

test: lundi.hpp test.cpp
	clang++ -std=c++11 -stdlib=libc++ test.cpp -o test -llua -I.

clean:
	rm test
