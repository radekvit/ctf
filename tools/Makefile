TOOLS = grammar

.PHONY: all format test pack doc clean $(TOOLS)
TARGET = all
all: $(TOOLS)

$(TOOLS):
	$(MAKE) -C $@ $(TARGET)

debug: TARGET=debug
debug: $(TOOLS)

format: TARGET=format
format: $(TOOLS)

test: TARGET=test
test: $(TOOLS)

pack: clean
	zip -r tools.zip $(TOOLS)

doc:
	$(MAKE) -C $(DOC)

clean: TARGET=clean
clean: $(TOOLS)
