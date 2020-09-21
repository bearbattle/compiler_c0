old_suffix = .c
new_suffix = .txt
.PHONY: txt


all: txt
	zip testfile.zip test*.txt

txt:
	$(shell ./change.sh)

1234:
	for (( var = 2; var < 5; var++ )); do
	    cp testfile1.txt testfile${var}.txt && \
    	cp testin1.txt testin${var}.txt && \
    	cp testout1.txt testout${var}.txt
	done