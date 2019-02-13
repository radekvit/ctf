SRC = src
INCLUDE = include
DOC = docs

.PHONY: all format test pack doc clean

all:

format:
	clang-format -style=file -i $(SRC)/*.hpp $(INCLUDE)/*.hpp
	$(MAKE) -C test format

test:
	$(MAKE) -C test test

pack: clean
	zip -r ctf.zip LICENSE.MIT README.md $(SRC)/*.hpp $(INCLUDE)/*.hpp doc media .clang-format Makefile test/Makefile test/*.cpp test/media test/ lib

doc:
	$(MAKE) -C $(DOC)

clean:
	-rm -r $(DOC)/html
