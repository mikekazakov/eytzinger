CC=clang++
CXXFLAGS=-std=c++14 -stdlib=libc++
INCLUDE=-I./fixed_eytzinger_map/include/ -I./external/Catch/include

all: fixed_eytzinger_map/tests/fixed_eytzinger_map_sanity_tests.cpp
	$(CC) $(CXXFLAGS) $(INCLUDE) -o tests fixed_eytzinger_map/tests/fixed_eytzinger_map_sanity_tests.cpp

test:
	./tests

clean:
	rm -f tests
