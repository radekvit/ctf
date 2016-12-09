APPNAME=bp
INCLUDEDIR=include
SRCDIR=src
CXXFLAGS+=-std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDEDIR)

HEADERS=$(INCLUDEDIR)/generic_types.h $(INCLUDEDIR)/ll_table.h \
$(INCLUDEDIR)/translation_grammar.h
OBJFILES=main.o translation_grammar.o

.PHONY: all format clean debug build

all: debug

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	-rm -f $(OBJFILES) $(APPNAME)

format:
	clang-format -style=file -i $(SRCDIR)/*.cpp $(INCLUDEDIR)/*.h
