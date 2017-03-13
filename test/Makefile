APPNAME=ctf_test
INCLUDE=../include
SRC=../src
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -I. -I $(INCLUDE)
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(INCLUDE)/generic_types.h $(INCLUDE)/ll_table.h \
$(INCLUDE)/translation_grammar.h
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))
OBJFILES += $(OBJ)/main.o

.PHONY: all format clean debug build test

all: debug

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

$(OBJ)/main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJFILES) $(APPNAME)

format:
	clang-format -style=file -i $(SRC)/*.cpp $(INCLUDE)/*.h

test: all
test:
	./$(APPNAME) && diff expected out
