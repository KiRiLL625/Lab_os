.PHONY: all mycat run clean
all: mycat
mycat: main.c
	gcc main.c -o mycat
run: mycat
	./mycat
clean:
	rm -f mycat