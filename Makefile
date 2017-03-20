CC = gcc
CXX = g++
CXXFLAGS = -std=c++11
SRC = ./src
OBJ = ./build/main.o ./build/runner.o ./build/submission.o ./build/utils.o
ISODIR = ./isolate/

build: mkdir ./bin/lxtester ./bin/isolate

./bin/lxtester: $(OBJ)
	$(CXX) $(CXXFLAGS) -o ./bin/lxtester $(OBJ) 

.PHONY: ./bin/isolate
./bin/isolate: 
	$(MAKE) -C $(ISODIR)
	mv ./isolate/isolate ./bin/isolate

./build/main.o: 
./build/runner.o:
./build/submission.o:
./build/utils.o:

.PHONY: mkdir
mkdir:
	mkdir -p ./bin
	mkdir -p ./build

.PHONY: clean
clean:
	-rm -rf ./build/*
	-rm -f ./bin/lxtester
	$(MAKE) -C $(ISODIR) clean

./build/%.o : ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
