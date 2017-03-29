SRC = src
INCLUDE = include

.PHONY: all format test pack doc

all:

format:
	clang-format -style=file -i $(SRC)/*.h $(INCLUDE)/*.h

test:
	make -C test test

pack:
	make -C test clean
	zip -r ctf.zip .

doc:
	make -C doc
