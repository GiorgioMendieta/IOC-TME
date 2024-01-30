NAME =thirion_mendieta# nom1_nom2
RPI ?=25# card_number

CROSS_COMPILE ?=bcm2708hardfp-

CFLAGS=-Wall -Wfatal-errors -O2
LDFLAGS=-static  

all: clean dep copy

help:
	@echo ""
	@echo "make [RPI=num] target"
	@echo ""
	@echo "      target :	help	this message"
	@echo " 		all	clean all and recompile"
	@echo " 		dep	compile if required"
	@echo " 		clean	clean all except sources"
	@echo " 		copy	compile then copy to card $(RPI) in directory $(NAME)"
	@echo ""


dep : blink0.x

copy: *.x
	@echo "Copying blink0.x to ~/$(NAME)"
	scp -P 622$(RPI) $^ pi@peri:$(NAME)

%.x: %.c
	$(CROSS_COMPILE)gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.x *~ *.a
