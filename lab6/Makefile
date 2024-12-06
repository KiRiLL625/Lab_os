.PHONY: all mypipe myfifo clean run

all: mypipe myfifo

mypipe: pipe/main.c
        gcc pipe/main.c -o mypipe

myfifo: fifo/main.c
        gcc fifo/main.c -o myfifo

clean:
        rm -f pipe/mypipe
        rm -f fifo/myfifo

run: mypipe myfifo
        ./pipe/mypipe & ./fifo/myfifo
