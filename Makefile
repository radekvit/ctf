SRC = src
INCLUDE = include
TOOLS = tools
DOC = docs

.PHONY: all format test pack doc clean

all:
	$(MAKE) -C $(TOOLS)

format:
	clang-format -style=file -i $(SRC)/*.hpp $(INCLUDE)/*.hpp
	$(MAKE) -C test format
	$(MAKE) -C $(TOOLS) format

test:
	$(MAKE) -C test test

pack: clean
	zip -r ctf.zip LICENSE.MIT README.md $(SRC)/*.hpp $(INCLUDE)/*.hpp media .clang-format Makefile test/Makefile test/*.cpp test/media test/ lib tools

doc:
	$(MAKE) -C $(DOC)

clean:
	$(MAKE) -C test clean
	$(MAKE) -C $(TOOLS) clean
	$(MAKE) -C $(DOC) clean
