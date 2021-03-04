.PHONY:all

all:test-rls test-debug

INCLUDE:=-I../..

test-rls:debug.cpp
	g++ $^ -o $@ $(INCLUDE)

test-debug:debug.cpp
	g++ $^ -o $@ -g $(INCLUDE)