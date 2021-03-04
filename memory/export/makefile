.PHONY:all

SO:=liba.so
EXE:=test

SOSRCS:=a.cpp
EXESRCS:=b.cpp

CPPFLAGS:=-g -Wall
INCLUDES:=-I../..

all:$(SO) $(EXE)

a.cpp:a.h
b.cpp:a.h

$(SO):$(SOSRCS)
	g++ $^ -o $@ -fPIC -shared $(CPPFLAGS) -DDEBUG $(INCLUDES)

$(EXE):$(EXESRCS) $(SO)
	g++ $(EXESRCS) -o $@ -L. -la $(INCLUDES)

run:export LD_LIBRARY_PATH=.
run:
	./$(EXE)

clean:
	rm -f $(SO) $(EXE)