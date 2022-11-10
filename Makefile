all: ringmaster player

ringmaster: ringmaster.c host.c host.h potato.h
	gcc -g -o ringmaster ringmaster.c host.c
player: player.c host.c host.h potato.h
	gcc -g -o player player.c host.c

.PHONY:
	clean
clean:
	rm -rf *.o ringmaster player