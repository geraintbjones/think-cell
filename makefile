EXE	:= interval_map
CXX := $(HOME)/local/gcc-13.2.0/bin/g++
# export LD_LIBRARY_PATH=/home/robert/local/gcc-13.2.0/lib64
CXXFLAGS =             \
    -ggdb              \
    -O0                \
    -Werror            \
    -Wextra            \
    -Wall              \
    -std=c++20         \
    -D_GLIBCXX_DEBUG   \
    -fsanitize=address \

.PHONY: clean all

all: $(EXE)

$(EXE):

clean:
	-rm -f $(EXE)

