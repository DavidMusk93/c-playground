.PHONY:all run clean check

EXE:=a.out
SO:=libtrivial.so
CPPFLAGS+=-Wall -g -rdynamic
INCLUDE:=-I..

all:$(EXE)

$(SO):extern.cpp
	g++ -fPIC -o $@ -shared $^ $(CPPFLAGS)

$(EXE):$(SO) double_free.cpp
	g++ double_free.cpp -L. -ltrivial $(CPPFLAGS) $(INCLUDE)

run:export LD_LIBRARY_PATH=.
run:
	./$(EXE)

clean:
	rm -f *.so core.* a.out logfile

check:
	@sysctl kernel.randomize_va_space
