# Directory to save files ~/nom1_nom2
DIR =thirion_mendieta
# Raspberry Pi Card Number
RPI ?=25
# Cross compiler
CROSS_COMPILE ?=bcm2708hardfp-

CFLAGS=-Wall -Wfatal-errors -O2
LDFLAGS=-static  

all: clean dep copy

help:
	@echo ""
	@echo "make [RPI=num] target"
	@echo ""
	@echo "     target :	help	this message"
	@echo "		all	clean all and recompile"
	@echo "		dep	compile if required"
	@echo "		clean	clean all except sources"
	@echo "		copy	compile then copy to card $(RPI) in directory $(DIR)"
	@echo ""


dep : blink0.x blink0_pt.x read_bp.x

copy: *.x
	@echo "Copying compiled files to ~/$(DIR)"
	scp -P 622$(RPI) $^ pi@peri:$(DIR)

%.x: %.c
	$(CROSS_COMPILE)gcc -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.x *~ *.a
