APPNAME=../grammarc
INCLUDE=.
SRC=.
CTF = ../../include
CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -I. -I $(CTF) -I $(INCLUDE) -I $(LIB)
OBJ=obj
LIB = ../lib/tclap-1.2.2/include
$(shell mkdir -p $(OBJ))

HEADERS=$(wildcard $(INCLUDE)/*.hpp)
OBJFILES=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))
DEPENDENCIES = $(OBJFILES:%.o=%.d)

.PHONY: all format clean debug deploy build test

all: deploy

build: $(APPNAME)

debug: CXXFLAGS += -g -O0
debug: build

deploy: CXXFLAGS += -O2
deploy: build

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $(LDLIBS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) -MMD -MP $(CXXFLAGS) -c $< -o $@

clean:
	-rm -rf $(OBJ) $(APPNAME)

format:
	clang-format -style=file -i main.cpp

-include $(DEPENDENCIES)