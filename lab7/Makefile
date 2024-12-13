.PHONY: all clean sender receiver

all: sender receiver

sender: sender.c
	gcc sender.c -o sender

receiver: receiver.c
	gcc receiver.c -o receiver

clean: rm -f sender receiver
