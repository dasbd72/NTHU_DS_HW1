CC = gcc
CXX = g++-11
CFLAGS = -pthread -fopenmp -O2
CFLAGS += -Wall -Wextra
CFLAGS += -DTIMING
CFLAGS += -DDEBUG
# CFLAGS += -DNOWRITE
# CFLAGS += -g -fsanitize=address
CXXFLAGS = -std=c++2a $(CFLAGS)

TARGETS = 109062131_hw1

.PHONY: all
all: $(TARGETS)

109062131_hw1: 109062131_hw1.cpp
	$(CXX) $(CFLAGS) -o 109062131_hw1 109062131_hw1.cpp

.PHONY: clean
clean:
	rm -f $(TARGETS)