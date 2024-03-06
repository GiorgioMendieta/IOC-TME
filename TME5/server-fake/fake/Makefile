APPS = fake
CFLAGS=-Wall -Werror
RPI=25
DIR=thirion_mendieta/server-fake/fake

all 	: $(APPS)
clean 	:;rm $(APPS) *~ f2s s2f 2> /dev/null || true
copy : $(APPS)
	@echo "Copying compiled files to ~/$(DIR)"
	scp -P 622$(RPI) $^ pi@peri:$(DIR)
