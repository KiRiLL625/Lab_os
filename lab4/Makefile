.PHONY: all clean run

all: mychmod
mychmod: main.c
        gcc main.c -o mychmod
clean:
        rm -f mychmod
run: mychmod
        ./mychmod 777 test
