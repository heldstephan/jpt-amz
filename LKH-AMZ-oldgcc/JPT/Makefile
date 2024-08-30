# CC=gcc
CFLAGS= -O3 -g -pedantic -Wall -Wstrict-prototypes -Wno-unused-result

all:
	$(MAKE) score

score: score.o JPTscore.o
	$(CC) $(CFLAGS) -o ../score score.o JPTscore.o -lm

clean:
	rm -f *.o ../score _* *~ ._*

score.o:    score.c    JPTutil.h
JPTscore.o: JPTscore.c JPTutil.h
