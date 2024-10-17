.PHONY: all clean myfork run

all: myfork

myfork: main.c
	gcc main.c -o myfork

run: myfork
	./myfork

clean:
	rm -f myfork
