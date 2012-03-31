all: test-ActionToken

test-ActionToken: test-ActionToken.cpp ActionToken.hpp
	g++ -std=c++0x -g -lpthread test-ActionToken.cpp -o test-ActionToken
