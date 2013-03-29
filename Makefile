all: test

test: lundi.hpp test.cpp
	clang++ test.cpp -o test -llua

clean:
	rm test
	