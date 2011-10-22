## Build flags
CFLAGS := -g3 
CPPFLAGS := -I. -std=c99 -DCOD_PLATFORM=COD_X11 -Wall -Wextra $(shell pkg-config --cflags x11)
LDFLAGS := -lcod $(shell pkg-config --libs x11) -lm -L.

## Build variables
OUT := libcod.a
SRC := common drawing font image stb-png x11
SRC := $(foreach x,$(SRC),src/$(x).c)
OBJ := $(patsubst src/%.c,src/.%.o, $(SRC))
DEP := $(patsubst src/%.c,src/.%.d, $(SRC))
EXAMPLES := examples/font examples/events examples/skeleton examples/primitives examples/image

# Source code for counting lines of code in the whole codebase
CLOC_SRC := $(SRC) src/win32.c src/cod.h

## Local settings
-include site.mk

## Patterns
src/.%.d: src/%.c
	@echo -n ' MM  ';
	$(strip $(CC) $(CPPFLAGS) -MM $< -MT $(patsubst %.d,%.o,$@) > $@)

src/.%.o: src/%.c
	@echo -n ' CC  ';
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<)

examples/%: examples/%.c $(OUT)
	@echo -n ' LD  ';
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS))

## Targets
all: $(OUT) $(EXAMPLES)

-include $(DEP)

lab: lab.c $(OUT)
	@echo -n ' LD  ';
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS))

$(OUT): $(OBJ)
	@echo -n ' AR  ';
	$(strip ar rcs $@ $^)

# Creates amalgamated file of all sources for ease of distribution
cod.c: $(SRC)
	cat $^ > $@

## Utilities
.PHONY: clean cleaner cloc

clean:
	@echo -n ' RM  ';
	rm -f $(wildcard $(OBJ) lab $(OUT) $(EXAMPLES) src/*.obj examples/*.obj)

cleaner: clean
	@echo -n ' RM  ';
	rm -f $(wildcard $(DEP) cod.c)

cloc:
	cloc $(CLOC_SRC)
	@echo 
	@echo ----------------------- WITH STB_IMAGE ------------------------
	@echo
	cloc $(CLOC_SRC) src/stb-png.c
