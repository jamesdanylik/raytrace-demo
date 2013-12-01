# A generic makefile

CC=g++
CFLAGS=-c -std=c++11 -Wall -Wno-unused-function
LDFLAGS=
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=raytrace-demo
ARCHIVE=403548957_linux

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm -rf $(OBJECTS) $(EXECUTABLE) $(ARCHIVE).zip *.ppm

run: $(EXECUTABLE)
	./$(EXECUTABLE) $(ARGS)

dist: clean
	zip -r $(ARCHIVE) ./*
