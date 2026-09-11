CXX = g++
CXXFLAGS = -std=c++17 -g -O0 -Wall -Wextra

.PHONY: all clean

all: build/app

build/app: build/main.o build/stats.o
	$(CXX) $(CXXFLAGS) build/main.o build/stats.o -o build/app

build/main.o: main.cpp stats.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c main.cpp -o build/main.o

build/stats.o: stats.cpp stats.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c stats.cpp -o build/stats.o

clean:
	rm -rf build
