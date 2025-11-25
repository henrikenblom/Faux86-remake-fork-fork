# Test Makefile for Faux86 i386 implementation

CXX = gcc
CXXFLAGS = -std=c++11 -Wall -Wextra -DCPU_386
LDFLAGS = -lstdc++

# Test targets
TESTS = test_registers

all: $(TESTS)
	@echo "Running all tests..."
	@for test in $(TESTS); do \
		echo ""; \
		./$$test || exit 1; \
	done

test_registers: test_registers.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TESTS) *.o

.PHONY: all clean
