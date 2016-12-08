.PHONY: all format clean format
format:
	clang-format -style=file -i *.cpp *.h