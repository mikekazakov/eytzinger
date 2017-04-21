ifndef CXX
CXX=clang++
endif
ifndef CXXFLAGS
CXXFLAGS=-std=c++11 -stdlib=libc++
endif
INCLUDE=-I./fixed_eytzinger_map/include/ -I./external/Catch/include

all: fixed_eytzinger_map/tests/fixed_eytzinger_map_sanity_tests.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o tests fixed_eytzinger_map/tests/fixed_eytzinger_map_sanity_tests.cpp

test:
	./tests

clean:
	rm -f tests
