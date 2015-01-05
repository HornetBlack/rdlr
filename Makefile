
# Currently fixed. In future that may change.
EXE := bin/rdl
EXE_SRC := src/*.c

# Build artifacts.

# The actual color value doesn't matter just that it exists
GCC_COLORS := auto
CC := gcc
CFLAGS := -Wall -Werror

.PHONY: all
all: check_dirs $(EXE)

$(EXE): $(EXE_SRC)
	gcc -Wall -Werror -o $@ $(EXE_SRC)

.PHONY: check_dirs
check_dirs:
	mkdir -p bin
	mkdir -p build

.PHONY: clean
clean:
	rm bin/*
	rmdir bin
	rm build/*
	rmdir build

