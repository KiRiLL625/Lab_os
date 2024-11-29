.PHONY: all clean run mypipe myfifo

all: mypipe myfifo

mypipe: pipe/main.c
        gcc pipe/main.c -o pipe/mypipe

myfifo: fifo/main.c
        gcc fifo/main.c -o fifo/myfifo

clean:
        rm -f pipe/mypipe fifo/myfifo

run: mypipe myfifo
        ./pipe/mypipe & ./fifo/myfifo
