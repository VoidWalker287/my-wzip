wzip: wzip.o utils.o config.h
	gcc -o wzip wzip.o utils.o -pthread

wzip2.o: wzip.c
	gcc -g -c wzip.c -Ofast

utils.o: utils.c
	gcc -g -c utils.c -Ofast
