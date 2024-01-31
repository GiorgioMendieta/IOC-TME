CC=bcm2708hardfp-gcc
CFLAGS=-O2 -static
CARD=25
NAME=thirion_mendieta
CFLAGS=-W -Wall -Wextra -Wfatal-errors -O2
APP=helloworld

all: $(APP)
	scp -P 622$(CARD) $^ pi@peri:$(NAME)

helloworld.x: helloworld.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm $(APP)
