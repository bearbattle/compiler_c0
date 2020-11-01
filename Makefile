.PHONY: txt

all: txt
	zip testfile.zip test*.txt

txt:
	$(shell ./change.sh)