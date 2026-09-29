CXX ?= g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -I. -I../../src

test: test_relay
	./test_relay

test_relay: test_relay.cpp ../../src/NovaHeap_Relay.cpp ../../src/NovaHeap_Relay.h Arduino.h
	$(CXX) $(CXXFLAGS) -o $@ test_relay.cpp ../../src/NovaHeap_Relay.cpp

clean:
	rm -f test_relay test_relay.exe

.PHONY: test clean
