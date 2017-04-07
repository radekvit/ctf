APPNAME=ctf_test
INCLUDE=../src
SRC=.
CATCH = ../lib/Catch/single_include
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -I. -I $(CATCH) -I $(INCLUDE)
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(INCLUDE)/*.h)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))

.PHONY: all format clean debug build test

all: debug

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

deploy: CXXFLAGS+=-O3 -DNDEBUG
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJ) $(APPNAME)

format:
	clang-format -style=file -i $(SRC)/*.cpp $(INCLUDE)/*.h

test: debug
test:
	./$(APPNAME) && make clean
