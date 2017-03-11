CC = gcc
CXX = g++
CXXFLAGS = -std=c++11
SRC = ./src
OBJ = ./build/main.o ./build/runner.o ./build/utils.o

build: mkdir ./bin/lxtester

./bin/lxtester: $(OBJ)
	$(CXX) $(CXXFLAGS) -o ./bin/lxtester $(OBJ) 

./build/main.o: 
./build/runner.o:
./build/utils.o:

.PHONY: mkdir
mkdir:
	mkdir -p ./bin
	mkdir -p ./build

.PHONY: clean
clean:
	-rm -rf ./build/*
	-rm -f ./bin/lxtester

./build/%.o : ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
