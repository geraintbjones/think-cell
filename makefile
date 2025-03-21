EXE	:= interval_map
CXX := $(HOME)/local/gcc-13.2.0/bin/g++
CXXFLAGS = -std=c++20 -D_GLIBCXX_DEBUG

.PHONY: clean all

all: $(EXE)

$(EXE):

clean:
	-rm -f $(EXE)
