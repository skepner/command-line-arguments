# -*- Makefile -*-
# Eugene Skepner 2015
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

SOURCES = test.cc

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic
  STD = c++14
else
  WEVERYTHING = -Wall -Wextra
  STD = c++14 # c++11
endif

WARNINGS = # -Wno-padded
OPTIMIZATION = # -O3
CXXFLAGS = -MMD -g $(OPTIMIZATION) -std=$(STD) $(WEVERYTHING) $(WARNINGS)
LDFLAGS =
LDLIBS =

# ----------------------------------------------------------------------

BUILD = build
DIST = dist

all: test

test: $(DIST)/test
	#$(DIST)/test aaa --pdf /tmp/a.pdf -x

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(DIST)/test: $(patsubst %.cc,$(BUILD)/%.o,$(SOURCES)) | $(DIST)
	g++ $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf $(DIST) $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: %.cc | $(BUILD)
	g++ $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

$(DIST):
	if [ ! -d "$@" ]; then mkdir -p "$@"; fi

$(BUILD):
	if [ ! -d "$@" ]; then mkdir -p "$@"; fi
