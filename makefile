EXE	:= interval_map
CXX := $(HOME)/local/gcc-13.2.0/bin/g++
CXXFLAGS =             \
    -ggdb              \
    -O0                \
    -Werror            \
    -Wextra            \
    -Wall              \
    -std=c++20         \
    -D_GLIBCXX_DEBUG   \
    -fsanitize=address \

.PHONY: clean all run

run: all
	LD_LIBRARY_PATH=$(HOME)/local/gcc-13.2.0/lib64 ./$(EXE) 10 | less
	
all: $(EXE)

$(EXE):

clean:
	-rm -f $(EXE)

