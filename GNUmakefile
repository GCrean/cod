## Build flags
CFLAGS := -g3 
CPPFLAGS := -I. -std=gnu99 -Wall -Wextra $(shell pkg-config --cflags x11) -DCOD_PLATFORM=COD_X11
LDFLAGS := -L. -lcod -lm $(shell pkg-config --libs x11)

## Source code

## Build variables
OUT := libcod.a
SRC := src/common.c src/drawing.c src/font.c src/image.c src/stb-png.c src/x11.c
OBJ := $(patsubst src/%.c,src/.%.o, $(SRC))
DEP := $(patsubst src/%.c,src/.%.d, $(SRC))
EXAMPLES := examples/font examples/eyes examples/events examples/image examples/primitives examples/skeleton

# Source code for counting lines of code in the whole codebase
CLOC_SRC := $(SRC) src/win32.c cod.h

## Local settings
-include site.mk

## Patterns
src/.%.d: src/%.c
	@echo -n ' MM  ';
	$(strip $(CC) $(CPPFLAGS) -MM $< -MT $(patsubst %.d,%.o,$@) > $@)

src/.%.o: src/%.c
	@echo -n ' CC  ';
	$(strip $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<)

src/.%.o: src/%.m
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

## Utilities
.PHONY: clean cleaner cloc cod-all.h

# Creates amalgamated file of all sources for ease of distribution
cod-all.h: cod.h src/common.c src/drawing.c src/font.c src/image.c src/stb-png.c src/x11.c src/win32.c src/cocoa.m
	cat cod.h > $@
	echo "#ifdef COD_LINK" >> $@
	echo "#ifdef __cplusplus" >> $@
	echo "extern \"C\" {" >> $@
	echo "#endif" >> $@
	cat src/common.c src/drawing.c src/font.c src/image.c src/stb-png.c src/x11.c src/win32.c src/cocoa.m >> $@
	echo "#ifdef __cplusplus" >> $@
	echo "}" >> $@
	echo "#endif" >> $@
	echo "#endif // COD_LINK" >> $@
	sed -i -e "s/#include \"cod.h\"//g;" $@
	rm -f cod-all.h-e

clean:
	@echo -n ' RM  ';
	rm -f $(wildcard $(OBJ) lab $(OUT) $(EXAMPLES) src/*.obj examples/*.obj examples/*.exe *.obj *.exe *.o)

cleaner: clean
	@echo -n ' RM  ';
	rm -f $(wildcard $(DEP) cod.c)
	rm -rf $(wildcard *dSYM)

cloc:
	cloc $(CLOC_SRC)
	@echo 
	@echo ----------------------- WITH STB_IMAGE ------------------------
	@echo
	cloc $(CLOC_SRC) src/stb-png.c
