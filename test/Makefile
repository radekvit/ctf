APPNAME=ctf_test
INCLUDE=../src
SRC=.
CATCH = ../lib/Catch/single_include/catch2
CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -I. -I $(CATCH) -I $(INCLUDE)
OBJ=obj
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(INCLUDE)/*.hpp)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))
DEPENDENCIES = $(OBJFILES:%.o=%.d)

.PHONY: all format clean debug build test

all: debug

build: $(APPNAME)

debug: CXXFLAGS+=-g -O0
debug: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) -MMD -MP $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJ) $(APPNAME)

format:
	clang-format -style=file -i *.cpp

test: all
test:
	./$(APPNAME)

-include $(DEPENDENCIES)
