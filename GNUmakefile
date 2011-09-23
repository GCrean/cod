## Build flags
CFLAGS := -g3 
CPPFLAGS := -Isrc -std=c99 -DCOD_PLATFORM=COD_X11 -Wall -Wextra $(shell pkg-config --cflags x11)
LDFLAGS := $(shell pkg-config --libs x11) -lm

## Build variables
EXE := lab
OUT := libcod.a
SRC := common font image x11
SRC := $(foreach x,$(SRC),src/$(x).c)
OBJ := $(patsubst src/%.c,src/.%.o, $(SRC))
DEP := $(patsubst src/%.c,src/.%.d, $(SRC))

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
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS) -L. -lcod)

## Targets
all: examples/events examples/skeleton examples/image lab

-include $(DEP)

lab: lab.c | $(OUT)
	@echo -n ' LD  ';
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS) -L. -lcod)

$(OUT): $(OBJ)
	@echo -n ' AR  ';
	$(strip ar rcs $@ $^)

## Utilities
.PHONY: clean cleaner

clean:
	@echo -n ' RM  ';
	rm -f $(wildcard $(OBJ) $(EXE) $(OUT) examples/image examples/skeleton)

cleaner: clean
	@echo -n ' RM  ';
	rm -f $(wildcard $(DEP))