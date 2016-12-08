APPNAME=bp
INCLUDEDIR=include
SRCDIR=src
CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDEDIR)

HEADERS=$(INCLUDEDIR)/generic_types.h $(INCLUDEDIR)/ll_table.h \
$(INCLUDEDIR)/translation_grammar.h
OBJFILES=translation_grammar.o

.PHONY: all format clean format

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	-rm -f $(OBJFILES) $(APPNAME)

format:
	clang-format -style=file -i $(SRCDIR)/*.cpp $(INCLUDEDIR)/*.h
