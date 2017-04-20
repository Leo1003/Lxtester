CC = gcc
CXX = g++
CXXFLAGS = -std=c++11
LIB = ./lib
LIBS = -L $(LIB) -lboost_date_time -lboost_random -lboost_system -lsioclient -lsioclient_tls -lpthread
SRC = ./src
OBJ = ./build/global.o ./build/main.o ./build/config.o ./build/runner.o ./build/submission.o ./build/testsuite.o ./build/utils.o ./build/server_socket.o ./build/logger.o
ISODIR = ./isolate/

build: mkdir ./bin/lxtester ./bin/isolate

./bin/lxtester: $(OBJ)
	$(CXX) $(CXXFLAGS) -o ./bin/lxtester $(OBJ) $(LIBS)

.PHONY: ./bin/isolate
./bin/isolate: 
	$(MAKE) -C $(ISODIR)
	cp ./isolate/isolate ./bin/isolate
	cp ./isolate/default.cf ./isolate.conf

./build/global.o:
./build/main.o: 
./build/config.o: 
./build/runner.o:
./build/submission.o:
./build/testsuite.o:
./build/utils.o:
./build/server_socket.o:
./build/logger.o:

.PHONY: mkdir
mkdir:
	mkdir -p ./bin
	mkdir -p ./build

.PHONY: clean
clean:
	-rm -rf ./build/*
	-rm -f ./bin/lxtester
	-rm ./isolate.conf
	$(MAKE) -C $(ISODIR) clean

./build/%.o : ./src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
