CC = gcc
CXX = g++
CXXFLAGS = -std=c++11
SRC = ./src

build: | ./bin/lxtester ./bin ./build

./bin/lxtester: ./build/main.o ./build/runner.o
	$(CXX) $(CXXFLAGS) -o ./bin/lxtester ./build/main.o

./build/main.o: ./src/main.cpp
	$(CXX) $(CXXFLAGS) -c -o ./build/main.o ./src/main.cpp

./build/runner.o: ./src/runner.cpp ./src/runner.h
	$(CXX) $(CXXFLAGS) -c -o ./build/runner.o ./src/runner.cpp
	
./bin:
	mkdir -p ./bin

./build:
	mkdir -p ./build

.PHONY: clean
clean:
	-rm -rf ./build/*
	-rm -f ./bin/lxtester
