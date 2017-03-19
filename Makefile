APPNAME=libctf.so
INCLUDE=include
SRC=src
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDE) -fpic
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(INCLUDE)/generic_types.h $(INCLUDE)/ll_table.h \
$(INCLUDE)/translation_grammar.h
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

.PHONY: all format clean debug build test pack doc

all: deploy

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) -shared $(CXXFLAGS) $(LDLIBS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJFILES) $(APPNAME) doc/html

format:
	clang-format -style=file -i $(SRC)/*.cpp $(INCLUDE)/*.h

test:
	make -C test test

pack: all
pack:
	zip ctf.zip include/*.h $(APPNAME)

doc:
	make -C doc
