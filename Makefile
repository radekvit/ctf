SRC = src
INCLUDE = include

.PHONY: all format test pack doc clean

all:

format:
	clang-format -style=file -i $(SRC)/*.hpp $(INCLUDE)/*.hpp

test:
	make -C test test

pack: clean
	zip -r ctf.zip LICENSE.MIT README.md $(SRC)/*.hpp $(INCLUDE)/*.hpp doc media .clang-format Makefile test/Makefile test/*.cpp test/media test/ lib

doc:
	make -C doc

clean:
	-rm -r doc/html