## Build flags
CFLAGS := -g3 
CPPFLAGS := -I. -std=c99 -DCOD_PLATFORM=COD_X11 -Wall -Wextra $(shell pkg-config --cflags x11)
LDFLAGS := $(shell pkg-config --libs x11) 

## Build variables
EXE := lab
SRC := cod
SRC := $(foreach x,$(SRC),$(x).c)
OBJ := $(patsubst %.c,.%.o, $(SRC))
DEP := $(patsubst %.c,.%.d, $(SRC))

## Local settings
-include site.mk

## Patterns
.%.d: %.c
	@echo -n ' MM  ';
	$(strip $(CC) $(CPPFLAGS) -MM $< -MT $(patsubst %.d,%.o,$@) > $@)

.%.o: %.c
	@echo -n ' CC  ';
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<)

## Targets
all: examples/image examples/skeleton $(EXE)

-include $(DEP)

examples/image: $(OBJ)
examples/skeleton: $(OBJ)

$(EXE): $(OBJ)
	@echo -n ' LD  ';
	$(strip $(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) lab.c)

## Utilities
.PHONY: clean cleaner

clean:
	@echo -n ' RM  ';
	rm -f $(wildcard $(OBJ) $(EXE))

cleaner: clean
	@echo -n ' RM  ';
	rm -f $(wildcard $(DEP))
