
# Currently fixed. In future that may change.
EXE := bin/rdl
EXE_SRC := $(wildcard src/*.c)

PLUGIN := $(wildcard src/*.rdl)
PLUGIN_LIB := $(PLUGIN:src/%.rdl=bin/%.so)
PLUGIN_SRC := $(wildcard src/*.rdl/*.c)

# The actual color value doesn't matter just that it exists
GCC_COLORS := auto
CC := gcc
CFLAGS := -Wall -Werror -O2

.PHONY: all
all: check_dirs $(EXE) $(PLUGIN_LIB)
	@ echo $(PLUGIN)
	@ echo $(PLUGIN_LIB)

$(EXE): $(EXE_SRC)
	$(CC) $(CFLAGS) -o $@ $(EXE_SRC) -ldl

bin/%.so: src/%.rdl/*.c 
	$(CC) -fPIC $(CFLAGS) -shared -o $@ $<

# Emacs tags
TAGS:
	etags $(EXE_SRC) $(PLUGIN_SRC)

.PHONY: check_dirs
check_dirs:
	mkdir -p bin
	mkdir -p build

.PHONY: clean
clean: check_dirs
	rm -r bin
	rm -r build

