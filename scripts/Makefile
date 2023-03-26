CC = gcc
CXX = g++
CFLAGS = -O3
CXXFLAGS = $(CFLAGS)

TARGETS = diff

.PHONY: all
all: $(TARGETS)

seq: seq.cpp
	g++-11 -std=c++2a -pthread -fopenmp -O2 -o seq seq.cpp

.PHONY: clean
clean:
	rm -f $(TARGETS)